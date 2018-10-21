/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#if defined(__OpenBSD__)
	#include <soundcard.h>
#else
	#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>

#include "../util.h"

const char *
vol_perc(const char *card)
{
	size_t i;
	int v, afd, devmask;
	char *vnames[] = SOUND_DEVICE_NAMES;

	if ((afd = open(card, O_RDONLY | O_NONBLOCK)) < 0) {
		warn("open '%s':", card);
		return NULL;
	}

	if (ioctl(afd, (int)SOUND_MIXER_READ_DEVMASK, &devmask) < 0) {
		warn("ioctl 'SOUND_MIXER_READ_DEVMASK':");
		close(afd);
		return NULL;
	}
	for (i = 0; i < LEN(vnames); i++) {
		if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
			if (ioctl(afd, MIXER_READ(i), &v) < 0) {
				warn("ioctl 'MIXER_READ(%ld)':", i);
				close(afd);
				return NULL;
			}
		}
	}

	close(afd);

	return bprintf("%d", v & 0xff);
}
