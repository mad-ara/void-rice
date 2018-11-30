/* Copyright 2017 Matteo Alessio Carrara <sw.matteoac@gmail.com> */

// TODO Cache
// TODO Read the .desktop standard
// TODO Copy functions from i3-dmenu-desktop and j4-dmenu-desktop

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#define die(...) {fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE);}

struct desktop_file
{
		char path[PATH_MAX];
		char *app_name;
		short name_len;
		char *exec_name;
		char **exec_args;
		off_t size;
};

// after the last execution, files can be realloc-ed to nfiles in order to free unnecessary memory
void find_files(const char *scandir, struct desktop_file **files, unsigned *nfiles)
{
	DIR *d;
	struct dirent *entry;
	struct stat estat;
	char *efullpath = malloc(PATH_MAX);

	static unsigned out_bufsiz = 200;
	static bool first_run = true;

	if (first_run)
	{
		first_run = false;
		*files = malloc(sizeof(struct desktop_file) * out_bufsiz);
		*nfiles = 0;

		if(*files == NULL)
		{
			fprintf(stderr, "Memory allocation error, cannot scan dir '%s': %s\n", scandir, strerror(errno));
			return;
		}
	}
	else
	{
		if(*files == NULL)
		{
			fprintf(stderr, "Invalid pointer, cannot scan dir '%s'\n", scandir);
			return;
		}
	}

	d = opendir(scandir);
	if (d == NULL)
	{
		perror(scandir);
		return;
	}

	while(1)
	{
ls_start:
		errno = 0;
		entry = readdir(d);
		if (entry == NULL)
		{
			if(errno == 0)
				break;
			else
			{
				perror(scandir);
				goto ls_start;
			}
		}

		strcpy(efullpath, scandir);
		strcat(efullpath, "/");
		strcat(efullpath, entry->d_name);

		if (stat(efullpath, &estat) == -1)
		{
			perror(efullpath);
			goto ls_start;
		}

		if (S_ISDIR(estat.st_mode))
		{
			if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
			{
				find_files(efullpath, files, nfiles);
			}
		}
		else if(strlen(entry->d_name) >= strlen(".desktop"))
		{
			if (!strcmp(entry->d_name + (strlen(entry->d_name) - strlen(".desktop")), ".desktop"))
			{
				(*files)[*nfiles].size = estat.st_size;
				strcpy((*files)[*nfiles].path, efullpath);
				(*nfiles)++;

				if (*nfiles == out_bufsiz)
				{
					struct desktop_file *tmp;
					out_bufsiz += 200;
					tmp = realloc(*files, sizeof(struct desktop_file) * out_bufsiz);

					if(tmp == NULL)
					{
						fprintf(stderr, "Memory allocation error, cannot scan dir '%s': %s\n", scandir, strerror(errno));
						return;
					}
					else *files = tmp;
				}
			}
		}
	}

	if (closedir(d) == -1) perror(scandir);
	free(efullpath);
}

int main (int argc, char **argv)
{
	char **dmenu_argv;
	char **searchdirs, *xdg_data_dirs, *home;
	short ndirs;

	char testdir[PATH_MAX];
	struct desktop_file *file;
	unsigned files_found;

	short fbufsize;
	char *fbuf;
	char *fbuftmp;
	int fd;

	char **oth_headers = NULL;
	short oth_headers_found;
	char *header;

	char **name_keys = NULL;
	char **names_start = NULL;
	short names_found;
	short name_idx;

	unsigned name_len;

	pid_t child_pid;
	int child_status;
	int child_stdin[2], child_stdout[2];

	char *uc;
	short uc_bufsiz, uc_len;
	ssize_t uc_read;

	// parse command line

	dmenu_argv = malloc(sizeof(char*) * 3);
	dmenu_argv[0] = "dmenu";
	dmenu_argv[1] = "-i";
	dmenu_argv[2] = NULL;

	if(argc > 1)
	{
		for(short i = 1; i < argc; i++)
		{
			if(argv[i][0] == '-')
			{
				if(!strcmp(argv[i], "--help"))
				{
					printf("Usage: k5-dmenu-desktop [--help] [--dmenu \"dmenu -i\"]\n");
					exit(EXIT_SUCCESS);
				}
				else if(!strcmp(argv[i], "--dmenu"))
				{
					if((i + 1) >= argc)
					{
						die("Missing parameter for '--dmenu'\n");
					}
					else
					{
						char *tmp;
						bool in_word;
						short args;

						i++;

						tmp = malloc(strlen(argv[i]) + 1);
						strcpy(tmp, argv[i]);

						in_word = false;
						args = 0;
						for(short j = 0; tmp[j] != '\0'; j++)
						{
							if(tmp[j] != ' ')
							{
								if(!in_word)
								{
									in_word = true;
									args++;
									dmenu_argv = realloc(dmenu_argv, sizeof(char*) * args);
									dmenu_argv[args - 1] = tmp + j;
								}
							}
							else
							{
								if(in_word)
								{
									in_word = false;
									tmp[j] = '\0';
								}
							}
						}

						if(args < 1)
						{
							die("An empty string has been used as an argument for --dmenu\n");
						}
						else
						{
							args++;
							dmenu_argv = realloc(dmenu_argv, sizeof(char*) * args);
							dmenu_argv[args - 1] = NULL;
						}
					}
				}
				else
				{
					goto invalid_cmdline;
				}
			}
			else
			{
invalid_cmdline:;
				die("Invalid argument: '%s'. Use '--help' for help.\n", argv[i]);
			}
		}
	}


	// find paths where to search for .desktop files

	ndirs = 1;
	searchdirs = malloc(sizeof(char*) * ndirs);
	searchdirs[ndirs - 1] = getenv("XDG_DATA_HOME");
	if (searchdirs[ndirs - 1] == NULL)
	{
		home = getenv("HOME");
		if (home == NULL)
		{
			fprintf(stderr, "Warning: Neither $XDG_DATA_HOME nor $HOME are set\n");
			free(searchdirs[ndirs - 1]);
			ndirs--;
		}
		else
		{
			searchdirs[ndirs - 1] = malloc(PATH_MAX);
			strcpy(searchdirs[ndirs - 1], home);
			strcat(searchdirs[ndirs - 1], "/.local/share");
		}
	}

	xdg_data_dirs = getenv("XDG_DATA_DIRS");
	if (xdg_data_dirs == NULL)
	{
		ndirs += 2;
		searchdirs = realloc(searchdirs, sizeof(char*) * ndirs);
		searchdirs[ndirs - 2] = "/usr/local/share";
		searchdirs[ndirs - 1] = "/usr/share";
	}
	else
	{
		char *tok = strtok(xdg_data_dirs, ":");
		while(tok != NULL)
		{
			ndirs++;
			searchdirs = realloc(searchdirs, sizeof(char*) * ndirs);
			searchdirs[ndirs - 1] = tok;
			tok = strtok(NULL, ":");
		}
	}

	// find .desktop files

	for(short i = 0; i < ndirs; i++)
	{
		strcpy(testdir, searchdirs[i]);
		strcat(testdir, "/applications");
		find_files(testdir, &file, &files_found);
	}
	file = realloc(file, sizeof(struct desktop_file) * files_found);
	if(files_found == 0) die("No files found\n");


	// find application names

	fbufsize = 20000;
	fbuf = malloc(fbufsize + 1);
	for(unsigned n = 0; n < files_found; n++)
	{
		file[n].app_name = NULL;

		fd = open(file[n].path, O_RDONLY);
		if (fd == -1)
		{
			perror(file[n].path);
			goto next_file;
		}

		if(file[n].size > fbufsize)
		{
			fbuftmp = realloc(fbuf, file[n].size + 1);

			if (fbuftmp == NULL)
			{
				fprintf(stderr, "%s: cannot process: %s\n", file[n].path, strerror(errno));
				goto next_file;
			}
			else
			{
				fbufsize = file[n].size;
				fbuf = fbuftmp;
			}
		}

		if(read(fd, fbuf, fbufsize) == -1)
		{
			perror(file[n].path);
			goto next_file;
		}

		fbuf[file[n].size] = '\0';

		// find the 'Desktop Entry' header

		oth_headers = NULL;
		oth_headers_found = 0;
		header = NULL;
		for(unsigned i = 0; fbuf[i] != '\0'; i++)
		{
			if(fbuf[i] == '[')
			{
				if(i > 0)
					if(fbuf[i - 1] != '\n')
						goto not_an_header;

				if(!strncmp(fbuf + i, "[Desktop Entry]", strlen("[Desktop Entry]")))
				{
					if(header == NULL)
						header = fbuf + i;
					else
					{
						fprintf(stderr, "%s: Duplicate 'Desktop Entry' header found\n", file[n].path);
						goto close_desktop_file;
					}
				}
				else
				{
					oth_headers_found++;
					oth_headers = realloc(oth_headers, sizeof(char*) * oth_headers_found);
					oth_headers[oth_headers_found - 1] = fbuf + i;
				}
			}
not_an_header:;
		}

		if(header == NULL)
		{
			if (oth_headers_found == 0)
				fprintf(stderr, "%s: No headers found\n", file[n].path);
			else
				fprintf(stderr, "%s: Cannot find the `Desktop Entry` header\n", file[n].path);

			goto close_desktop_file;
		}

		// find the 'Name' key

		name_keys = NULL;
		names_start = NULL;
		names_found = 0;
		name_idx = -1;
		for(unsigned i = 0; i < (file[n].size - strlen("Name=")); i++)
		{
			if (fbuf[i] == 'N')
			{
				if(i > 0)
				{
					if(fbuf[i - 1] != '\n')
						goto not_a_name;
				}

				if(!strncmp(fbuf + i, "Name", strlen("Name")))
				{
					for(unsigned j = i + strlen("Name"); fbuf[j] != '\0'; j++)
					{
						if((fbuf[j] != ' ') && (fbuf[j] != '='))
							goto not_a_name;

						if (fbuf[j] == '=')
						{
							names_found++;
							name_keys = realloc(name_keys, sizeof(char*) * names_found);
							names_start = realloc(names_start, sizeof(char*) * names_found);

							name_keys[names_found - 1] = fbuf + i;
							names_start[names_found - 1] = fbuf + j + 1;

							i = j + 1;
							while(1)
							{
								if((fbuf[i] == '\n') || (fbuf[i] == '\0'))
									break;
								i++;
							}

							break;
						}
					}
				}
			}
not_a_name:;
		}

		if (names_found == 0)
		{
			fprintf(stderr, "%s: no 'Name' keys\n", file[n].path);
			goto close_desktop_file;
		}

		// if we are here we found the header and at least one 'Name' key

		if(oth_headers_found == 0)
		{
			if (names_found == 1)
			{
				if(name_keys[0] > header)
				{
					name_idx = 0;
				}
				else
				{
					fprintf(stderr, "%s: 'Name' key found but incorrect position\n", file[n].path);
					goto close_desktop_file;
				}
			}
			else
			{
				fprintf(stderr, "%s: Invalid syntax, found multiple `Name` keys ‎under the same header\n", file[n].path);
				goto close_desktop_file;
			}
		}
		else
		{
			if (names_found == 1)
			{
				if (name_keys[0] > header)
				{
					// verify that the key is under the correct header

					for(short i = 0; i < oth_headers_found; i++)
					{
						if((oth_headers[i] > header) && (name_keys[0] > oth_headers[i]))
						{
							fprintf(stderr, "%s: 'Name' key found but under incorrect header\n", file[n].path);
							goto close_desktop_file;
						}
					}

					name_idx = 0;
				}
				else
				{
					fprintf(stderr, "%s: 'Name' key found but under incorrect header\n", file[n].path);
					goto close_desktop_file;
				}
			}
			else
			{
				for(short i = 0; i < names_found; i++)
				{
					if(name_keys[i] > header)
					{
						for(short j = 0; j < oth_headers_found; j++)
						{
							if((oth_headers[j] > header) && (name_keys[i] > oth_headers[j]))
								goto try_next_name;
						}

						if(name_idx != -1)
						{
							fprintf(stderr, "%s: Invalid syntax, found multiple `Name` keys ‎under the same header\n", file[n].path);
							goto close_desktop_file;
						}
						else
						{
							name_idx = i;
						}
					}
try_next_name:;
				}

				if(name_idx == -1)
				{
					fprintf(stderr, "%s: 'Name' keys found but under incorrect header\n", file[n].path);
					goto close_desktop_file;
				}
			}
		}

		// find the Exec key
		// FIXME Check the header

		for(unsigned i = 0; i < (file[n].size - strlen("Exec=")); i++)
		{
			if(fbuf[i] == 'E')
			{
				if(i > 0)
					if(fbuf[i - 1] != '\n')
						goto not_exec_key;

				if(!strncmp(fbuf + i, "Exec", strlen("Exec")))
				{
					unsigned exec_start = i + strlen("Exec");
					unsigned exec_len;

					for(;;exec_start++)
					{
						if(fbuf[exec_start] != ' ')
						{
							if(fbuf[exec_start] == '=')
							{
								exec_start++;
								break;
							}
							else
								goto not_exec_key;
						}
					}

					for(exec_len = 0; (fbuf[exec_start + exec_len] != '\n') && (fbuf[exec_start + exec_len] != '\0'); exec_len++);

					char *tmp =  malloc(exec_len + 1);
					strncpy(tmp, fbuf + exec_start, exec_len);
					tmp[exec_len] = '\0';

					char *tok = strtok(tmp, " ");
					if(tok == NULL)
					{
						fprintf(stderr, "%s: Invalid exec value\n", file[n].path);
						goto close_desktop_file;
					}
					else
					{
						file[n].exec_name = malloc(strlen(tok) + 1);
						strcpy(file[n].exec_name, tok);
					}

					file[n].exec_args = NULL;
					short argn = 0;

					argn++;
					file[n].exec_args = realloc(file[n].exec_args, sizeof(char*) * argn);
					file[n].exec_args[argn - 1] = malloc(strlen(file[n].exec_name) + 1);
					strcpy(file[n].exec_args[argn - 1], file[n].exec_name);

					while((tok = strtok(NULL, " ")) != NULL)
					{
						if(tok[0] != '%')
						{
							argn++;
							file[n].exec_args = realloc(file[n].exec_args, sizeof(char*) * argn);
							file[n].exec_args[argn - 1] = malloc(strlen(tok) + 1);
							strcpy(file[n].exec_args[argn - 1], tok);
						}
					}

					argn++;
					file[n].exec_args = realloc(file[n].exec_args, sizeof(char*) * argn);
					file[n].exec_args[argn - 1] = NULL;
				}
			}
not_exec_key:;
		}

		// keys found

		for(name_len = 0; (names_start[name_idx][name_len] != '\n') && (names_start[name_idx][name_len] != '\0'); name_len++);

		file[n].name_len = name_len;
		file[n].app_name = malloc(name_len + 1);
		strncpy(file[n].app_name, names_start[name_idx], name_len);
		file[n].app_name[name_len] = '\0';

close_desktop_file:
		if(close(fd) == -1) perror(file[n].path);
		free(oth_headers);
		free(name_keys);
		free(names_start);
next_file:;
	}

	// create the dmenu child process

	pipe(child_stdin);
	pipe(child_stdout);
	child_pid = fork();

	if(child_pid == -1)
	{
		perror("Cannot create dmenu process");
		exit(EXIT_FAILURE);
	}
	else if(child_pid == 0) // child process code
	{
		close(child_stdin[1]);
		close(child_stdout[0]);

		dup2(child_stdin[0], 0);
		dup2(child_stdout[1], 1);

		if(execvp(dmenu_argv[0], dmenu_argv) == -1)
		{
			perror("Cannot create dmenu process");
			exit(EXIT_FAILURE);
		}
	}
	else // parent process code
	{
		close(child_stdin[0]);
		close(child_stdout[1]);

		// sort application by name
		// TODO add option to disable
		for(unsigned i = 0; i < files_found - 1; i++)
		{
			if (file[i].app_name != NULL)
			{
				for(unsigned j = i + 1; j < files_found; j++)
				{
					if(file[j].app_name != NULL)
					{
						if(strcmp(file[i].app_name, file[j].app_name) > 0)
						{
							struct desktop_file tmp = file[i];
							file[i] = file[j];
							file[j] = tmp;
						}
					}
				}
			}
		}

		// send application names to dmenu
		// since the executable can be chosen by the user, it may not read anything
		// from stdin; thus generating an error
		signal(SIGPIPE, SIG_IGN);
		for(unsigned i = 0; i < files_found; i++)
		{
			if(file[i].app_name != NULL)
			{
				if(write(child_stdin[1], file[i].app_name, file[i].name_len) == -1)
					break;
				if(write(child_stdin[1], "\n", 1) == -1)
					break;
			}
		}
		close(child_stdin[1]);
		signal(SIGPIPE, SIG_DFL);

		if(waitpid(child_pid, &child_status, 0) == -1)
		{
			perror(NULL);
			exit(EXIT_FAILURE);
		}

		if(WIFEXITED(child_status))
			if(WEXITSTATUS(child_status) == EXIT_SUCCESS)
				goto child_ok;

		die("Error in child process\n");

child_ok:;
	}


	// get the user's choice

	uc_len = 0;
	uc_bufsiz = 100;
	uc = NULL;

	do
	{
		uc = realloc(uc, uc_bufsiz + 1);
		uc_read = read(child_stdout[0], uc, uc_bufsiz);
		uc_len += uc_read;
	} while(uc_read >= uc_bufsiz);
	uc[uc_read - 1] = '\0'; // dmenu inserts a newline at the end


	// run the program
	child_pid = fork();

	if(child_pid == -1)
	{
		// FIXME add the name of process
		die("Cannot start child process: %s\n", strerror(errno));
	}
	else if(child_pid == 0) // child process code
	{
		for(unsigned i = 0; i < files_found; i++)
		{
			if(file[i].app_name != NULL)
			{
				if(!strcmp(file[i].app_name, uc))
				{
					if(execvp(file[i].exec_name, file[i].exec_args) == -1)
					{
						die("Cannot start child process: %s\n", strerror(errno));
					}
				}
			}
		}
		die("No application is called '%s'\n", uc);
	}
	else // parent process code
	{
		return 0;
	}

}
