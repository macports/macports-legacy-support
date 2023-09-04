/*
 * Copyright (C) 2023 raf <raf@raf.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

int main(int ac, char **av)
{
	int failures = 0;
	uid_t real_uid = getuid();
	uid_t effective_uid = geteuid();
	uid_t real_gid = getgid();
	uid_t effective_gid = getegid();

	// When supplied with arguments, compare faccessat() against access().
	// We can't test AT_EACCESS here because access() can't do that.
	// But some tests will be setuid, some setgid, and some neither.
	// See test/do_test_faccessat_setuid for details.

	if (ac > 1)
	{
		// For each file, test all modes in isolation and in all combinations

		int modes[8] = { F_OK, R_OK, W_OK, X_OK, R_OK|W_OK, W_OK|X_OK, X_OK|R_OK, R_OK|W_OK|X_OK };
		int a, m;

		for (a = 1; a < ac; ++a)
		{
			for (m = 0; m < 8; ++m)
			{
				int faccessat_rc = faccessat(AT_FDCWD, av[a], modes[m], 0);
				int faccessat_errno = errno;
				int access_rc = access(av[a], modes[m]);
				int access_errno = errno;

				if (faccessat_rc != access_rc || (faccessat_rc < 0 && faccessat_errno != access_errno))
				{
					if (!failures)
						fprintf(stderr, "faccessat_setuid: uid=%s euid=%s gid=%s egid=%s\n",
							getpwuid(real_uid) ? getpwuid(real_uid)->pw_name : "?",
							getpwuid(effective_uid) ? getpwuid(effective_uid)->pw_name : "?",
							getgrgid(real_gid) ? getgrgid(real_gid)->gr_name : "?",
							getgrgid(effective_gid) ? getgrgid(effective_gid)->gr_name : "?"
						);

					++failures;

					fprintf(stderr, "faccessat(%s, %s%s%s%s) fail: %d %s != %d %s\n",
						av[a],
						(modes[m] == F_OK) ? "F" : "",
						(modes[m] & R_OK) ? "R" : "",
						(modes[m] & W_OK) ? "W" : "",
						(modes[m] & X_OK) ? "X" : "",
						faccessat_rc,
						(faccessat_rc == -1) ? strerror(faccessat_errno) : "",
						access_rc,
						(access_rc == -1) ? strerror(access_errno) : ""
					);

					char buf[1024];
					snprintf(buf, 1024, "/bin/ls -l %s", av[a]);
					system(buf);
				}
			}
		}

		unlink(av[0]);

		return (failures) ? 1 : 0;
	}

	// Without arguments, just create readable/writable/executable/inaccessible
	// files and test faccessat() with and without AT_EACCESS. This is part of
	// the automatic tests, but as part of manual testing might be setuid.
	// Instead of comparing against access(), these tests have explicit expected
	// results.

	#define TMP "test/tmp"
	const char *readable_path = TMP "/readable";
	const char *writable_path = TMP "/writable";
	const char *executable_path = TMP "/executable";
	const char *inaccessible_path = TMP "/inaccessible";
	const char *nonexistent_path = TMP "/nonexistent";

	/* Create test/tmp directory if not already there (see make test_faccessat_setuid) */

	int mkdir_rc = mkdir(TMP, 0700);
	int mkdir_errno = errno;

	if (mkdir_rc == -1 && mkdir_errno != EEXIST)
	{
		fprintf(stderr, "failed to create tmp directory %s: %s\n", TMP, strerror(errno));
		return 1;
	}

	/* Create readable/writable/executable/inaccessible files (owned by effective user) */

	int readable_fd = creat(readable_path, 0400);
	if (readable_fd == -1)
	{
		fprintf(stderr, "failed to create readable file %s: %s\n", readable_path, strerror(errno));
		if (mkdir_rc == 0)
			rmdir(TMP);
		return 1;
	}

	close(readable_fd);

	int writable_fd = creat(writable_path, 0200);
	if (writable_fd == -1)
	{
		fprintf(stderr, "failed to create writable file %s: %s\n", writable_path, strerror(errno));
		unlink(readable_path);
		if (mkdir_rc == 0)
			rmdir(TMP);
		return 1;
	}

	close(writable_fd);

	int executable_fd = creat(executable_path, 0100);
	if (executable_fd == -1)
	{
		fprintf(stderr, "failed to create executable file %s: %s\n", executable_path, strerror(errno));
		unlink(readable_path);
		unlink(writable_path);
		if (mkdir_rc == 0)
			rmdir(TMP);
		return 1;
	}

	close(executable_fd);

	int inaccessible_fd = creat(inaccessible_path, 0000);
	if (inaccessible_fd == -1)
	{
		fprintf(stderr, "failed to create inaccessible file %s: %s\n", inaccessible_path, strerror(errno));
		unlink(readable_path);
		unlink(writable_path);
		unlink(executable_path);
		if (mkdir_rc == 0)
			rmdir(TMP);
		return 1;
	}

	close(inaccessible_fd);

	/* Test faccessat() with no flags and with AT_EACCESS */

	int dfd = open(".", O_RDONLY); errno = 0;
	int readable_rok = faccessat(dfd, readable_path, R_OK, 0);
	int readable_rok_errno = errno; errno = 0;
	int writable_wok = faccessat(dfd, writable_path, W_OK, 0);
	int writable_wok_errno = errno; errno = 0;
	int executable_xok = faccessat(dfd, executable_path, X_OK, 0);
	int executable_xok_errno = errno; errno = 0;
	int inaccessible_rok = faccessat(dfd, inaccessible_path, R_OK, 0);
	int inaccessible_rok_errno = errno; errno = 0;
	int inaccessible_wok = faccessat(dfd, inaccessible_path, W_OK, 0);
	int inaccessible_wok_errno = errno; errno = 0;
	int inaccessible_xok = faccessat(dfd, inaccessible_path, X_OK, 0);
	int inaccessible_xok_errno = errno; errno = 0;
	int readable_erok = faccessat(dfd, readable_path, R_OK, AT_EACCESS);
	int readable_erok_errno = errno; errno = 0;
	int writable_ewok = faccessat(dfd, writable_path, W_OK, AT_EACCESS);
	int writable_ewok_errno = errno; errno = 0;
	int executable_exok = faccessat(dfd, executable_path, X_OK, AT_EACCESS);
	int executable_exok_errno = errno; errno = 0;
	int inaccessible_erok = faccessat(dfd, inaccessible_path, R_OK, AT_EACCESS);
	int inaccessible_erok_errno = errno; errno = 0;
	int inaccessible_ewok = faccessat(dfd, inaccessible_path, W_OK, AT_EACCESS);
	int inaccessible_ewok_errno = errno; errno = 0;
	int inaccessible_exok = faccessat(dfd, inaccessible_path, X_OK, AT_EACCESS);
	int inaccessible_exok_errno = errno; errno = 0;

	int readable_fok = faccessat(dfd, readable_path, F_OK, 0);
	int readable_fok_errno = errno; errno = 0;
	int writable_fok = faccessat(dfd, writable_path, F_OK, 0);
	int writable_fok_errno = errno; errno = 0;
	int executable_fok = faccessat(dfd, executable_path, F_OK, 0);
	int executable_fok_errno = errno; errno = 0;
	int inaccessible_fok = faccessat(dfd, inaccessible_path, F_OK, 0);
	int inaccessible_fok_errno = errno; errno = 0;
	int readable_efok = faccessat(dfd, readable_path, F_OK, AT_EACCESS);
	int readable_efok_errno = errno; errno = 0;
	int writable_efok = faccessat(dfd, writable_path, F_OK, AT_EACCESS);
	int writable_efok_errno = errno; errno = 0;
	int executable_efok = faccessat(dfd, executable_path, F_OK, AT_EACCESS);
	int executable_efok_errno = errno; errno = 0;
	int inaccessible_efok = faccessat(dfd, inaccessible_path, F_OK, AT_EACCESS);
	int inaccessible_efok_errno = errno; errno = 0;

	int nonexistent_fok = faccessat(dfd, nonexistent_path, F_OK, 0);
	int nonexistent_fok_errno = errno; errno = 0;
	int nonexistent_efok = faccessat(dfd, nonexistent_path, F_OK, AT_EACCESS);
	int nonexistent_efok_errno = errno; errno = 0;
	close (dfd);

	#define TEST(cond, message, e) if (!(cond)) { errno = 0; fprintf(stderr, "%s %s\n", message, (e) ? strerror(e) : ""); ++failures; }

	/* Test F_OK */

	TEST(readable_fok == 0, "root: faccessat(readable F_OK) does not exist = FAIL", readable_fok_errno)
	TEST(writable_fok == 0, "root: faccessat(writable F_OK) does not exist = FAIL", writable_fok_errno)
	TEST(executable_fok == 0, "root: faccessat(executable F_OK) does not exist = FAIL", executable_fok_errno)
	TEST(inaccessible_fok == 0, "root: faccessat(inaccessible F_OK) does not exist = FAIL", inaccessible_fok_errno)
	TEST(readable_efok == 0, "root: faccessat(readable F_OK AT_EACCESS) does not exist = FAIL", readable_efok_errno)
	TEST(writable_efok == 0, "root: faccessat(writable F_OK AT_EACCESS) does not exist = FAIL", writable_efok_errno)
	TEST(executable_efok == 0, "root: faccessat(executable F_OK AT_EACCESS) does not exist = FAIL", executable_efok_errno)
	TEST(inaccessible_efok == 0, "root: faccessat(inaccessible F_OK AT_EACCESS) does not exist = FAIL", inaccessible_efok_errno)
	TEST(nonexistent_fok == -1, "root: faccessat(nonexistent F_OK) does exist = FAIL", nonexistent_fok_errno)
	TEST(nonexistent_efok == -1, "root: faccessat(nonexistent F_OK AT_EACCESS) does exist = FAIL", nonexistent_efok_errno)

	/* Test non-setuid program */

	if (real_uid == effective_uid)
	{
		/* Test as non-root user */

		if (real_uid != 0)
		{
			TEST(readable_rok == 0, "not root: faccessat(readable R_OK) is unreadable = FAIL", readable_rok_errno)
			TEST(writable_wok == 0, "not root: faccessat(writable W_OK) is unwritable = FAIL", writable_wok_errno)
			TEST(executable_xok == 0, "not root: faccessat(executable X_OK) is unexecutable = FAIL", executable_xok_errno)
			TEST(inaccessible_rok == -1, "not root: faccessat(inaccessible R_OK) is readable = FAIL", inaccessible_rok_errno)
			TEST(inaccessible_wok == -1, "not root: faccessat(inaccessible W_OK) is writable = FAIL", inaccessible_wok_errno)
			TEST(inaccessible_xok == -1, "not root: faccessat(inaccessible X_OK) is executable = FAIL", inaccessible_xok_errno)
			/* This isn't meaningful because uid == euid */
			TEST(readable_erok == 0, "not root: faccessat(readable R_OK AT_EACCESS) is unreadable = FAIL", readable_erok_errno)
			TEST(writable_ewok == 0, "not root: faccessat(writable W_OK AT_EACCESS) is unwritable = FAIL", writable_ewok_errno)
			TEST(executable_exok == 0, "not root: faccessat(executable X_OK AT_EACCESS) is unexecutable = FAIL", executable_exok_errno)
			TEST(inaccessible_erok == -1, "not root: faccessat(inaccessible R_OK AT_EACCESS) is readable = FAIL", inaccessible_erok_errno)
			TEST(inaccessible_ewok == -1, "not root: faccessat(inaccessible W_OK AT_EACCESS) is writable = FAIL", inaccessible_ewok_errno)
			TEST(inaccessible_exok == -1, "not root: faccessat(inaccessible X_OK AT_EACCESS) is executable = FAIL", inaccessible_exok_errno)
		}

		/* Test as root user */

		else
		{
			TEST(readable_rok == 0, "root: faccessat(readable R_OK) is unreadable = FAIL", readable_rok_errno)
			TEST(writable_wok == 0, "root: faccessat(writable W_OK) is unwritable = FAIL", writable_wok_errno)
			TEST(executable_xok == 0, "root: faccessat(executable X_OK) is unexecutable = FAIL", executable_xok_errno)
			TEST(inaccessible_rok == 0, "root: faccessat(inaccessible R_OK) is unreadable = FAIL", inaccessible_rok_errno)
			TEST(inaccessible_wok == 0, "root: faccessat(inaccessible W_OK) is unwritable = FAIL", inaccessible_wok_errno)
			TEST(inaccessible_xok == -1, "root: faccessat(inaccessible X_OK) is executable = FAIL", inaccessible_xok_errno)
			/* This isn't meaningful because uid == euid */
			TEST(readable_erok == 0, "root: faccessat(readable R_OK AT_EACCESS) is unreadable = FAIL", readable_erok_errno)
			TEST(writable_ewok == 0, "root: faccessat(writable W_OK AT_EACCESS) is unwritable = FAIL", writable_ewok_errno)
			TEST(executable_exok == 0, "root: faccessat(executable X_OK AT_EACCESS) is unexecutable = FAIL", executable_exok_errno)
			TEST(inaccessible_erok == 0, "root: faccessat(inaccessible R_OK AT_EACCESS) is unreadable = FAIL", inaccessible_erok_errno)
			TEST(inaccessible_ewok == 0, "root: faccessat(inaccessible W_OK AT_EACCESS) is unwritable = FAIL", inaccessible_ewok_errno)
			TEST(inaccessible_exok == -1, "root: faccessat(inaccessible X_OK AT_EACCESS) is executable = FAIL", inaccessible_exok_errno)
		}
	}

	/* Test setuid root program */
	/* Note: Must be setuid root, run as non-root, linked using absolute path */

	else if (real_uid != 0 && effective_uid == 0)
	{
		TEST(readable_rok == -1, "setuid root: non-root user: faccessat(readable R_OK) is readable = FAIL", readable_rok_errno)
		TEST(writable_wok == -1, "setuid root: non-root user: faccessat(writable W_OK) is writable = FAIL", writable_wok_errno)
		TEST(executable_xok == -1, "setuid root: non-root user: faccessat(executable X_OK) is executable = FAIL", executable_xok_errno)
		TEST(inaccessible_rok == -1, "setuid root: non-root user: faccessat(inaccessible R_OK) is readable = FAIL", inaccessible_rok_errno)
		TEST(inaccessible_wok == -1, "setuid root: non-root user: faccessat(inaccessible W_OK) is writable = FAIL", inaccessible_wok_errno)
		TEST(inaccessible_xok == -1, "setuid root: non-root user: faccessat(inaccessible X_OK) is executable = FAIL", inaccessible_xok_errno)
		/* This is meaningful because uid != euid */
		TEST(readable_erok == 0, "setuid root: non-root user: faccessat(readable R_OK AT_EACCESS) is unreadable = FAIL", readable_erok_errno)
		TEST(writable_ewok == 0, "setuid root: non-root user: faccessat(writable W_OK AT_EACCESS) is unwritable = FAIL", writable_ewok_errno)
		TEST(executable_exok == 0, "setuid root: non-root user: faccessat(executable X_OK AT_EACCESS) is unexecutable = FAIL", executable_exok_errno)
		TEST(inaccessible_erok == 0, "setuid root: non-root user: faccessat(inaccessible R_OK AT_EACCESS) is unreadable = FAIL", inaccessible_erok_errno)
		TEST(inaccessible_ewok == 0, "setuid root: non-root user: faccessat(inaccessible W_OK AT_EACCESS) is unwritable = FAIL", inaccessible_ewok_errno)
		TEST(inaccessible_exok == -1, "setuid root: non-root user: faccessat(inaccessible X_OK AT_EACCESS) is executable = FAIL", inaccessible_exok_errno)
	}

	/* Test setuid non-root program */
	/* Note: Must be setuid, run as non-root other user, linked using absolute path */

	else if (real_uid != 0 && effective_uid != real_uid)
	{
		TEST(readable_rok == -1, "setuid non-root: non-root other user: faccessat(readable R_OK) is readable = FAIL", readable_rok_errno)
		TEST(writable_wok == -1, "setuid non-root: non-root other user: faccessat(writable W_OK) is writable = FAIL", writable_wok_errno)
		TEST(executable_xok == -1, "setuid non-root: non-root other user: faccessat(executable X_OK) is executable = FAIL", executable_xok_errno)
		TEST(inaccessible_rok == -1, "setuid non-root: non-root other user: faccessat(inaccessible R_OK) is readable = FAIL", inaccessible_rok_errno)
		TEST(inaccessible_wok == -1, "setuid non-root: non-root other user: faccessat(inaccessible W_OK) is writable = FAIL", inaccessible_wok_errno)
		TEST(inaccessible_xok == -1, "setuid non-root: non-root other user: faccessat(inaccessible X_OK) is executable = FAIL", inaccessible_xok_errno)
		/* This is meaningful because uid != euid */
		TEST(readable_erok == 0, "setuid non-root: non-root other user: faccessat(readable R_OK AT_EACCESS) is unreadable = FAIL", readable_erok_errno)
		TEST(writable_ewok == 0, "setuid non-root: non-root other user: faccessat(writable W_OK AT_EACCESS) is unwritable = FAIL", writable_ewok_errno)
		TEST(executable_exok == 0, "setuid non-root: non-root other user: faccessat(executable X_OK AT_EACCESS) is unexecutable = FAIL", executable_exok_errno)
		TEST(inaccessible_erok == -1, "setuid non-root: non-root other user: faccessat(inaccessible R_OK AT_EACCESS) is readable = FAIL", inaccessible_erok_errno)
		TEST(inaccessible_ewok == -1, "setuid non-root: non-root other user: faccessat(inaccessible W_OK AT_EACCESS) is writable = FAIL", inaccessible_ewok_errno)
		TEST(inaccessible_exok == -1, "setuid non-root: non-root other user: faccessat(inaccessible X_OK AT_EACCESS) is executable = FAIL", inaccessible_exok_errno)
	}

	/* Report any unexpected situation (won't happen) */

	else
	{
		fprintf(stderr, "This uid/euid combination isn't currently tested.\n");
		++failures;
	}

	/* Only show the test files and process permissions if there were failures */

	if (failures)
	{
		fprintf(stderr, "faccessat_setuid: uid=%s euid=%s gid=%s egid=%s\n",
			getpwuid(real_uid) ? getpwuid(real_uid)->pw_name : "?",
			getpwuid(effective_uid) ? getpwuid(effective_uid)->pw_name : "?",
			getgrgid(real_gid) ? getgrgid(real_gid)->gr_name : "?",
			getgrgid(effective_gid) ? getgrgid(effective_gid)->gr_name : "?"
		);

		system("/bin/ls -l test/tmp/*ble");
	}

	/* Test argument validation */

	int check_pathname_rc = faccessat(AT_FDCWD, NULL, R_OK, 0);
	int check_pathname_errno = errno;
	int check_dirfd_rc = faccessat(-1, "pathname", R_OK, 0);
	int check_dirfd_errno = errno;
	int check_mode_rc = faccessat(AT_FDCWD, "pathname", -1, 0); // Apple doesn't check this
	int check_mode_errno = errno;
	int check_flag_rc = faccessat(AT_FDCWD, "pathname", R_OK, -1);
	int check_flag_errno = errno;

	TEST(check_pathname_rc == -1, "check pathname failed", 0)
	if (check_pathname_rc == -1)
		TEST(check_pathname_errno == EFAULT, "check pathname errno wrong (should be EFAULT)", check_pathname_errno)
	TEST(check_dirfd_rc == -1, "check dirfd failed", 0)
	if (check_dirfd_rc == -1)
		TEST(check_dirfd_errno == EBADF, "check dirfd errno wrong (should be EBADF)", check_dirfd_errno)
	TEST(check_mode_rc == -1, "check mode failed", 0) // Apple doesn't check this argument - failure is ENOENT/EPERM
	//if (check_mode_rc == -1)
	//	TEST(check_mode_errno == EINVAL, "check mode errno wrong (should be EINVAL)", check_mode_errno)
	if (check_mode_rc == -1) // On 10.14 this is ENOENT. On 10.6 it's EPERM
		TEST(check_mode_errno == ENOENT || check_mode_errno == EPERM, "check mode errno wrong (should be ENOENT or EPERM)", check_mode_errno)
	TEST(check_flag_rc == -1, "check flag failed", 0)
	if (check_flag_rc == -1)
		TEST(check_flag_errno == EINVAL, "check flag errno wrong (should be EINVAL)", check_flag_errno)

	/* Cleanup */

	unlink(readable_path);
	unlink(writable_path);
	unlink(executable_path);
	unlink(inaccessible_path);
	if (mkdir_rc == 0)
		rmdir(TMP);

	/* Delete test/test_faccessat_setuid. Don't wait for make clean. */

	if (real_uid != effective_uid || real_gid != effective_gid)
		unlink(av[0]);

	return (failures) ? 1 : 0;
}

/* vi:set noet ts=4 sw=4: */
