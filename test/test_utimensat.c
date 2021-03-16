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
  * NOTICE: This file was modified by Ken Cunningham in September 2020 and Mihai
  * Moldovan in February 2021 to allow for use as a supporting file for MacPorts legacy
  * support library. This notice is included in support of clause 2.2 (b) of the Apple
  * Public License, Version 2.0.
  */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * darwintest seems to be private.
 * No source code could be located for it.
 *
 * However, there seems to be a different but compatible implementation in
 * xnu-6153.141.1/osfmk/tests/ktest.{c,h}.
 *
 * http://web.archive.org/web/20210201212906/https://opensource.apple.com/source/xnu/xnu-6153.141.1/osfmk/tests/ktest.h.auto.html
 *
 * Usually, darwintests would provide a dynamic library called darwintest_utils which
 * includes... who knows what exactly.
 *
 * ktest seems to be more complicated and tightly integrated.
 *
 * Long story short: we won't even try to reimplement the full darwintest suite here.
 * We can, however, take some of the ktest definitions as inspirations for what the
 * macros we actually use are supposed to do.
 */
/*
#include <darwintest.h>
#include <darwintest_utils.h>
*/


#define FILENAME "utimensat"

#define T_LOG(...)			\
	do {				\
		printf("Testing: ");	\
		printf(__VA_ARGS__);	\
		printf("\n");		\
	} while (0)

/*
 * Yes, we know that b can't be NULL when we use it, but some compilers (most notably older GCC,
 * but also more recent versions like 8) wrongly assume that it could be, triggering a bug and
 * superfluous warning.
 *
 * Work around that locally.
 */
#define T_ASSERT_POSIX_ZERO(a,b)				\
	do {							\
		if ((a) != 0) {					\
			printf("assert zero failed");		\
			if (b != NULL) {			\
				printf(": %s", (b) ? (b) : "");	\
			}					\
			printf("\n");				\
			goto err;				\
		}						\
	} while (0)

#define T_ASSERT_POSIX_SUCCESS(a,b)				\
	do {							\
		if ((a) < 0) {					\
			printf("assert success failed");	\
			if ((b) != NULL) {			\
				printf(": %s", (b) ? (b) : "");	\
			}					\
			printf("\n");				\
			goto err;				\
		}						\
	} while (0)

#define T_ASSERT_GE(a,b,c)							\
	do {									\
		if (!((a) >= (b))) {						\
			printf("assert GE failed {%ld, %ld}", (a), (b));	\
			if ((c) != NULL) {					\
				printf(": %s", (c) ? (c) : "");			\
			}							\
			printf("\n");						\
			goto err;						\
		}								\
	} while (0)

#define T_ASSERT_EQ(a,b,c)							\
	do {									\
		if (!((a) == (b))) {						\
			printf("assert EQ failed {%ld, %ld}", (a), (b));	\
			if ((c) != NULL) {					\
				printf(": %s", (c) ? (c) : "");			\
			}							\
			printf("\n");						\
			goto err;						\
		}								\
	} while (0)

#define T_SKIP(msg)						\
	do {							\
		printf("Test skipped");				\
		if ((msg)) {					\
			printf(": %s", (msg) ? (msg) : "");	\
		}						\
		printf("\n");					\
		goto out;					\
	} while (0)

#define T_DECL(a,b) int main(void)
#define T_SETUPBEGIN
#define T_SETUPEND
#define T_QUIET
#define dt_tmpdir tmpdir


struct bug_for_bug {
  size_t pos;
  bool enable;
};

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
	/* T_ASSERT_POSIX_ZERO(chdir(dt_tmpdir()), NULL); */
	// Skip the test if the current working directory is not on APFS.
	struct statfs sfs = { 0 };
	bool apfs = true;
	T_QUIET; T_ASSERT_POSIX_SUCCESS(statfs(".", &sfs), NULL);
	if (memcmp(&sfs.f_fstypename[0], "apfs", strlen("apfs")) != 0) {
		apfs = false;
		/* T_SKIP("utimensat is APFS-only, but working directory is non-APFS"); */
	}
	T_SETUPEND;

	struct stat pre_st, post_st, utimes_st;
	int fd, ret = EXIT_SUCCESS;
	struct bug_for_bug mtime_omit = {
					  5,
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050
	/*
	 * Older OS X versions do not seem to exhibit this bug.
	 *
	 * I've looked through the XNU kernel code and tried to diff the relevant
	 * sections, including the HFS part, from 10.5 and 10.6 without success.
	 *
	 * I cannot explain what is triggering it and why it's only triggered on HFS+
	 * for OS X >= 10.6.
	 *
	 * My personal speculation is that this is an effect of some sort of caching or
	 * the fact that atimes are only updated lazily. Though, if explicitly requested
	 * by user-space and set to a custom time, this laziness shouldn't play a role.
	 *
	 * We'll take the shortcut here and disable the bug-for-bug compatibility for
	 * older systems. We could, alternatively, emulate the buggy behavior in
	 * utimensat (or the setattrlistat emulation), but that wouldn't sound smart.
	 */
					  false
#else
					  (!(apfs))
#endif
	};


	T_ASSERT_POSIX_SUCCESS((fd = open(FILENAME, O_CREAT|O_RDWR, 0644)), NULL);
	T_ASSERT_POSIX_ZERO(close(fd), NULL);

	if (!(apfs)) {
		T_ASSERT_POSIX_SUCCESS((fd = open(FILENAME "-utimes", O_CREAT|O_RDWR, 0644)), NULL);
		T_ASSERT_POSIX_ZERO(close(fd), NULL);
	}

	for (size_t i = 0; i < sizeof(tptr)/sizeof(tptr[0]); i++) {
		T_LOG("=== {%ld, %ld} {%ld, %ld} ===",
				tptr[i][0].tv_sec, tptr[i][0].tv_nsec,
				tptr[i][1].tv_sec, tptr[i][1].tv_nsec);

		struct timespec now;
		struct timeval utimes_tv[2] = { { 0 } };

		/* Convert current entry to microseconds-based structure. */
		TIMESPEC_TO_TIMEVAL(utimes_tv, tptr[i]);
		TIMESPEC_TO_TIMEVAL((utimes_tv + 1), (tptr[i] + 1));

		clock_gettime(CLOCK_REALTIME, &now);

		T_ASSERT_POSIX_ZERO(stat(FILENAME, &pre_st), "first stat failed");
		T_ASSERT_POSIX_ZERO(utimensat(AT_FDCWD, FILENAME, tptr[i], 0), "utimensat failed");
		T_ASSERT_POSIX_ZERO(stat(FILENAME, &post_st), "second stat failed");

		if (!(apfs)) {
			/*
			 * Handle UTIME_NOW and UTIME_OMIT... uh... "correctly".
			 * By "correctly", we really mean to copy the file and then call
			 * utimes() on it.
			 *
			 * The issue is that utimes() cannot generally handle the
			 * UTIME_NOW/UTIME_OMIT macros (at least on older systems),
			 * so we'll have to operate on a copy of the original file.
			 *
			 * Avoid copyfile(), though, since it is not available on older
			 * systems. Instead, copy the timestamps directly via utimes().
			 */
			struct timeval utimes_tv_copy[2] = { { 0 } };
			TIMESPEC_TO_TIMEVAL(utimes_tv_copy, &(pre_st.st_atimespec));
			TIMESPEC_TO_TIMEVAL((utimes_tv_copy + 1), &(pre_st.st_mtimespec));
			T_ASSERT_POSIX_ZERO(utimes(FILENAME "-utimes", utimes_tv_copy), "copying original timestamps via utimes failed");

			/* Then, set the actual times. */
			T_ASSERT_POSIX_ZERO(utimes(FILENAME "-utimes", utimes_tv), "utimes failed");
			T_ASSERT_POSIX_ZERO(stat(FILENAME "-utimes", &utimes_st), "utimes stat failed");
		}

		if (tptr[i][0].tv_nsec == UTIME_NOW) {
			T_ASSERT_GE(post_st.st_atimespec.tv_sec, now.tv_sec, "post stat vs. now seconds (utimensat Atime nanoseconds UTIME_NOW)");
		} else if (tptr[i][0].tv_nsec == UTIME_OMIT) {
			T_ASSERT_EQ(post_st.st_atimespec.tv_sec, pre_st.st_atimespec.tv_sec, "post stat vs. pre stat seconds (utimensat Atime nanoseconds UTIME_OMIT)");
			T_ASSERT_EQ(post_st.st_atimespec.tv_nsec, pre_st.st_atimespec.tv_nsec, "post stat vs. pre stat nanoseconds (utimensat Atime nanoseconds UTIME_OMIT)");
		} else {
			/* Seconds must always match. */
			if ((mtime_omit.enable) && (mtime_omit.pos == i)) {
				/*
				 * Unless we're on non-APFS and the mtime is set to
				 * UTIME_OMIT, which triggers a bug in Apple's
				 * implementation (even natively on 10.13 and higher),
				 * which leads to the atime not being updated as well.
				 */
				T_ASSERT_EQ(pre_st.st_atimespec.tv_sec, post_st.st_atimespec.tv_sec, "pre stat vs. post stat Atime seconds (utimensat Atime explicit) - "
												     "if this test fails on 10.14 or higher, it means that Apple fixed "
												     "its buggy implementation on non-APFS file systems. Please report "
												     "this to the legacy-support project");
			}
			else {
				T_ASSERT_EQ(post_st.st_atimespec.tv_sec, tptr[i][0].tv_sec, "post stat vs. utimensat Atime seconds (utimensat Atime explicit)");
			}

			/* Nanoseconds, on the other hand... may not. */
			if (apfs) {
				/* But must on APFS. */
				T_ASSERT_EQ(post_st.st_atimespec.tv_nsec, tptr[i][0].tv_nsec, "post stat vs. utimensat Atime nanoseconds (utimensat Atime explicit)");
			}
			else {
				/*
				 * On other file systems, we just make sure the
				 * nanoseconds are within microseconds range compared to
				 * the utimes() call.
				 */
				T_ASSERT_EQ((post_st.st_atimespec.tv_nsec / 1000), (utimes_st.st_atimespec.tv_nsec / 1000), "post stat vs. utimes Atime nanoseconds (utimensat Atime explicit)");
			}
		}

		if (tptr[i][1].tv_nsec == UTIME_NOW) {
			T_ASSERT_GE(post_st.st_mtimespec.tv_sec, now.tv_sec, "post stat vs. now seconds (utimensat Mtime nanoseconds UTIME_NOW)");
		} else if (tptr[i][1].tv_nsec == UTIME_OMIT) {
			T_ASSERT_EQ(post_st.st_mtimespec.tv_sec, pre_st.st_mtimespec.tv_sec, "post stat vs. pre stat seconds (utimensat Mtime nanseconds UTIME_OMIT)");
			T_ASSERT_EQ(post_st.st_mtimespec.tv_nsec, pre_st.st_mtimespec.tv_nsec, "post stat vs. pre stat nanoseconds (utimensat Mtime nanoseconds UTIME_OMIT)");
		} else {
			/* Same here, but for mtime. */
			T_ASSERT_EQ(post_st.st_mtimespec.tv_sec, tptr[i][1].tv_sec, "post stat vs. utimensat Mtime seconds (utimensat Mtime explicit)");

			if (apfs) {
				T_ASSERT_EQ(post_st.st_mtimespec.tv_nsec, tptr[i][1].tv_nsec, "post stat vs. utimensat Mtime nanoseconds (utimensat Mtime explicit)");
			}
			else {
				T_ASSERT_EQ((post_st.st_mtimespec.tv_nsec / 1000), (utimes_st.st_mtimespec.tv_nsec / 1000), "post stat vs. utimes Mtime nanoseconds (utimensat Mtime explicit)");
			}
		}
	}

out:
	unlink(FILENAME);
	unlink(FILENAME "-utimes");
	return ret;
err:
	ret = EXIT_FAILURE;
	goto out;
}
