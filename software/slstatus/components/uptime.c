/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../util.h"

const char *
uptime(void)
{
	uintmax_t h, m;
	struct timespec uptime;

	if (clock_gettime(CLOCK_BOOTTIME, &uptime) < 0) {
		warn("clock_gettime 'CLOCK_BOOTTIME'");
		return NULL;
	}

	h = uptime.tv_sec / 3600;
	m = uptime.tv_sec % 3600 / 60;

	return bprintf("%juh %jum", h, m);
}
