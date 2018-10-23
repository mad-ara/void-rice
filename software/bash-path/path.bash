#!/usr/bin/env bash
#
# Functions to modify colon separated variables like $PATH or $MANPATH
#
# Author: Dave Eddy <dave@daveeddy.com>
# Date: September 07, 2018
# License: MIT

#
# path_add <path> [direction] [varname]
#
# path_add takes a directory name, an optional direction name (defaults to
# "after" to append to list) and an optional variable name (defaults to PATH)
# and adds the directory name to data stored in variable.
#
# Example
#
# given: PATH='/bin:/sbin'
#
# path_add /usr/bin
# // PATH => '/bin:/sbin:/usr/bin'
# path_add /opt/local/bin before
# // PATH => '/opt/local/bin:/bin:/sbin:/usr/bin'
#
# The variable name should be passed by name.
# foo=''
# path_add /bin after foo
# // foo => '/bin'
#
path_add() {
	local var=${3:-PATH}
	local path=
	path=$(path_print_add "$@") || return 1
	read -r "${var?}" <<< "$path"
}

#
# path_remove <path> [varname]
#
# path_remove takes a directory name and an optional variable name (defaults to
# PATH) and removes every instance of the directory name from the data stored in
# the variable.
#
# Example
#
# given: PATH='/bin:/sbin:/usr/bin:/usr/sbin'
#
# path_remove /usr/bin
# // PATH => '/bin:/sbin:/usr/sbin'
# path_remove /not-found
# // PATH => '/bin:/sbin:/usr/sbin'
#
# The variable name should be passed by name.
# foo='/bin:/sbin'
# path_remove /bin foo
# // foo => '/sbin'
#
path_remove() {
	local var=${2:-PATH}
	local path=
	path=$(path_print_remove "$@") || return 1
	read -r "${var?}" <<< "$path"
}

#
# path_clean [varname]
#
# path_clean takes an optional variable name (defaults to PATH) and "cleans" it,
# this process will:
#
# 1. Remove empty elements.
# 2. Remove relative directories.
# 3. Remove directories that don't exist/can't be accessed (checked with `cd`).
# 4. Remove duplicates (first element stays, subsequent elements are tossed).
#
# Example
#
# PATH='::/bin:/sbin:::./:../../some-path::/doesnt-exist'
# path_clean
# // PATH => '/bin:/sbin'

# PATH='/bin:/bin//:////bin//////:/bin/dir/..::'
# path_clean
# // PATH => '/bin'
#
# The variable name should be passed by name.
# foo='/bin:/bin'
# path_clean /bin foo
# // foo => '/bin'
#
path_clean() {
	local var=${1:-PATH}
	local path=
	path=$(path_print_clean "$@") || return 1
	read -r "${var?}" <<< "$path"
}

#
# Exact same usage as path_add but prints the new PATH only and doesn't modify
# anything in place.
#
path_print_add() {
	local p=$1
	local dir=${2:-after}
	local var=${3:-PATH}
	local arr
	declare -A seen

	if [[ -z $p || $p == *:* ]]; then
		echo "path_print_add: invalid argument: '$p'" >&2
		return 1
	fi

	IFS=: read -ra arr <<< "${!var}:"

	case "$dir" in
		after) arr=("${arr[@]}" "$p");;
		*) arr=("$p" "${arr[@]}");;
	esac

	local IFS=:
	echo "${arr[*]}"
}

#
# Exact same usage as path_remove but prints the new PATH only and doesn't
# modify anything in place.
#
path_print_remove() {
	local p=$1
	local var=${2:-PATH}
	local arr
	local newarr=()
	declare -A seen

	if [[ -z $p || $p == *:* ]]; then
		echo "path_print_remove: invalid argument: '$p'" >&2
		return 1
	fi

	IFS=: read -ra arr <<< "${!var}:"

	local _p
	for _p in "${arr[@]}"; do
		if [[ $p == "$_p" ]]; then
			continue
		fi
		newarr+=("$_p")
	done

	local IFS=:
	echo "${newarr[*]}"
}

#
# Exact same usage as path_clean but prints the new PATH only and doesn't
# modify anything in place.
#
path_print_clean() {
	local var=${1:-PATH}
	local arr
	local newarr=()
	declare -A seen

	# read PATH into an array, trailing ":" is due to:
	# http://mywiki.wooledge.org/BashPitfalls#pf47
	IFS=: read -ra arr <<< "${!var}:"

	local p
	for p in "${arr[@]}"; do
		# Empty element is equivalent to CWD (weird, I know), remove it
		if [[ -z $p ]]; then
			continue
		fi

		# Remove any relative paths
		if [[ ${p:0:1} != '/' ]]; then
			continue
		fi

		# Normalize path and ensure we can access it
		p=$(cd "$p" &>/dev/null && echo "$PWD")

		# Path doesn't exist or we can't access it
		if [[ -z $p ]]; then
			continue
		fi

		# Filter out dups while we are here
		if [[ -n ${seen[$p]} ]]; then
			continue
		fi
		seen[$p]=true

		# Store the new path
		newarr+=("$p")
	done

	local IFS=:
	echo "${newarr[*]}"
}
