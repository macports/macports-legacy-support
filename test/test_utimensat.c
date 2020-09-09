/*
 * Copyright (c) 2006, 2017 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

 /*
  * NOTICE: This file was modified by Ken Cunningham in September 2020 to allow
  * for use as a supporting file for MacPorts legacy support library. This notice
  * is included in support of clause 2.2 (b) of the Apple Public License,
  * Version 2.0.
  */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <darwintest.h>
#include <darwintest_utils.h>

#define FILENAME "utimensat"

static const struct timespec tptr[][2] = {
	{ { 0x12345678, 987654321 }, { 0x15263748, 123456789 }, },

	{ { 0, UTIME_NOW }, { 0x15263748, 123456789 }, },
	{ { 0x12345678, 987654321 }, { 0, UTIME_NOW }, },
	{ { 0, UTIME_NOW }, { 0, UTIME_NOW }, },

	{ { 0, UTIME_OMIT }, { 0x15263748, 123456789 }, },
	{ { 0x12345678, 987654321 }, { 0, UTIME_OMIT }, },
	{ { 0, UTIME_OMIT }, { 0, UTIME_OMIT }, },

	{ { 0, UTIME_NOW }, { 0, UTIME_OMIT }, },
	{ { 0, UTIME_OMIT }, { 0, UTIME_NOW }, },
};

T_DECL(utimensat, "Try various versions of utimensat")
{
	T_SETUPBEGIN;
	T_ASSERT_POSIX_ZERO(chdir(dt_tmpdir()), NULL);
	// Skip the test if the current working directory is not on APFS.
	struct statfs sfs = { 0 };
	T_QUIET; T_ASSERT_POSIX_SUCCESS(statfs(".", &sfs), NULL);
	if (memcmp(&sfs.f_fstypename[0], "apfs", strlen("apfs")) != 0) {
		T_SKIP("utimensat is APFS-only, but working directory is non-APFS");
	}
	T_SETUPEND;

	struct stat pre_st, post_st;
	int fd;

	T_ASSERT_POSIX_SUCCESS((fd = open(FILENAME, O_CREAT|O_RDWR, 0644)), NULL);
	T_ASSERT_POSIX_ZERO(close(fd), NULL);

	for (size_t i = 0; i < sizeof(tptr)/sizeof(tptr[0]); i++) {
		T_LOG("=== {%ld, %ld} {%ld, %ld} ===",
				tptr[i][0].tv_sec, tptr[i][0].tv_nsec,
				tptr[i][1].tv_sec, tptr[i][1].tv_nsec);

		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);

		T_ASSERT_POSIX_ZERO(stat(FILENAME, &pre_st), NULL);
		T_ASSERT_POSIX_ZERO(utimensat(AT_FDCWD, FILENAME, tptr[i], 0), NULL);
		T_ASSERT_POSIX_ZERO(stat(FILENAME, &post_st), NULL);

		if (tptr[i][0].tv_nsec == UTIME_NOW) {
			T_ASSERT_GE(post_st.st_atimespec.tv_sec, now.tv_sec, NULL);
		} else if (tptr[i][0].tv_nsec == UTIME_OMIT) {
			T_ASSERT_EQ(post_st.st_atimespec.tv_sec, pre_st.st_atimespec.tv_sec, NULL);
			T_ASSERT_EQ(post_st.st_atimespec.tv_nsec, pre_st.st_atimespec.tv_nsec, NULL);
		} else {
			T_ASSERT_EQ(post_st.st_atimespec.tv_sec, tptr[i][0].tv_sec, NULL);
			T_ASSERT_EQ(post_st.st_atimespec.tv_nsec, tptr[i][0].tv_nsec, NULL);
		}

		if (tptr[i][1].tv_nsec == UTIME_NOW) {
			T_ASSERT_GE(post_st.st_mtimespec.tv_sec, now.tv_sec, NULL);
		} else if (tptr[i][1].tv_nsec == UTIME_OMIT) {
			T_ASSERT_EQ(post_st.st_mtimespec.tv_sec, pre_st.st_mtimespec.tv_sec, NULL);
			T_ASSERT_EQ(post_st.st_mtimespec.tv_nsec, pre_st.st_mtimespec.tv_nsec, NULL);
		} else {
			T_ASSERT_EQ(post_st.st_mtimespec.tv_sec, tptr[i][1].tv_sec, NULL);
			T_ASSERT_EQ(post_st.st_mtimespec.tv_nsec, tptr[i][1].tv_nsec, NULL);
		}
	}
}
