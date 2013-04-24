/* Measure nanosleep relative timer latency
 *              by: john stultz (john.stultz@linaro.org)
 *		(C) Copyright Linaro 2013
 *              Licensed under the GPLv2
 *
 *  To build:
 *	$ gcc nsleep-rel-lat.c -o nsleep-rel-lat -lrt
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <string.h>
#include <signal.h>

#define NSEC_PER_SEC 1000000000ULL

#define UNRESONABLE_LATENCY 40000000 /* 40ms in nanosecs */


#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3
#define CLOCK_MONOTONIC_RAW		4
#define CLOCK_REALTIME_COARSE		5
#define CLOCK_MONOTONIC_COARSE		6
#define CLOCK_BOOTTIME			7
#define CLOCK_REALTIME_ALARM		8
#define CLOCK_BOOTTIME_ALARM		9
#define CLOCK_HWSPECIFIC		10
#define CLOCK_TAI			11
#define NR_CLOCKIDS			12

#define UNSUPPORTED 0xf00f

char *clockstring(int clockid)
{
	switch (clockid) {
	case CLOCK_REALTIME:
		return "CLOCK_REALTIME";
	case CLOCK_MONOTONIC:
		return "CLOCK_MONOTONIC";
	case CLOCK_PROCESS_CPUTIME_ID:
		return "CLOCK_PROCESS_CPUTIME_ID";
	case CLOCK_THREAD_CPUTIME_ID:
		return "CLOCK_THREAD_CPUTIME_ID";
	case CLOCK_MONOTONIC_RAW:
		return "CLOCK_MONOTONIC_RAW";
	case CLOCK_REALTIME_COARSE:
		return "CLOCK_REALTIME_COARSE";
	case CLOCK_MONOTONIC_COARSE:
		return "CLOCK_MONOTONIC_COARSE";
	case CLOCK_BOOTTIME:
		return "CLOCK_BOOTTIME";
	case CLOCK_REALTIME_ALARM:
		return "CLOCK_REALTIME_ALARM";
	case CLOCK_BOOTTIME_ALARM:
		return "CLOCK_BOOTTIME_ALARM";
	case CLOCK_TAI:
		return "CLOCK_TAI";
	};
	return "UNKNOWN_CLOCKID";
}

long long timespec_sub(struct timespec a, struct timespec b)
{
	long long ret = NSEC_PER_SEC * b.tv_sec + b.tv_nsec;
	ret -= NSEC_PER_SEC * a.tv_sec + a.tv_nsec;
	return ret;
}

int nanosleep_lat_test(int clockid)
{
	struct timespec start, end, target;
	int i, count;

	target.tv_sec = 0;
	target.tv_nsec = 1;

	if (clock_gettime(clockid, &start))
		return UNSUPPORTED;
	if (clock_nanosleep(clockid, 0, &target, NULL))
		return UNSUPPORTED;

	count = 1000;

	clock_gettime(clockid, &start);
	for (i=0;i<count;i++)
		clock_nanosleep(clockid, 0, &target, NULL);
	clock_gettime(clockid, &end);

	if ((timespec_sub(start, end)/count) > UNRESONABLE_LATENCY)
		return -1;

	return 0;
}

int main(int argc, char** argv)
{
	long long length;
	int failed = 0, clockid, ret;
	for (clockid = CLOCK_REALTIME; clockid < NR_CLOCKIDS; clockid++) {

		/* Skip cputime clockids since nanosleep won't increment cputime */
		if (clockid == CLOCK_PROCESS_CPUTIME_ID ||
				clockid == CLOCK_THREAD_CPUTIME_ID ||
				clockid == CLOCK_HWSPECIFIC)
			continue;

		printf("Nanosleep %-31s: ", clockstring(clockid));

		ret = nanosleep_lat_test(clockid);
		if (ret == UNSUPPORTED) {
			printf("UNSUPPORTED\n");
			continue;
		}
		if (ret < 0) {
			printf("FAILED\n");
			return -1;
		}
		printf("PASSED\n");
	}
	return 0;
}
