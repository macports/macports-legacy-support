#!/bin/sh

# Copyright (C) 2023 raf <raf@raf.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# When run as root, test faccessat() properly

# Note: On 10.4, this doesn't work because setreuid/setregid are incorrect.
# They set effective uid/gid, but not real uid/gid.

if [ "$(whoami)" != root ]
then
	echo 'Run "sudo make test_faccessat_setuid" to test faccessat properly'
	exit 1
fi

# Define some names and functions

s=tbin/test_faccessat_setuid
t=tst_data/tmp
fail=0

die() { echo "$0: $@" >&2; rm -rf $s $t; exit 1; }

setid() # usage: setid owner group mode fname [createcmd]
{
	if [ -n "$5" ]; then $5 $4 || die "$5 $4 failed"; fi
	chown $1 $4 || die "chown $1 $4 failed"
	chgrp $2 $4 || die "chgrp $2 $4 failed"
	chmod $3 $4 || die "chmod $3 $4 failed"
}

setup() # usage: setup owner group mode
{
	cp tbin/test_faccessat_static $s || die "cp $s failed"
	setid $1 $2 $3 $s
	setid $1 $2 $3 $t mkdir
}

check()
{
	"$@" || fail=1
}

clean()
{
	rm -rf $s $t
}

get_group() # Get the user's primary group name
{
	echo $(id $1 | sed -E 's/^.*gid=-?[0-9]+\(//; s/\).*$//')
}

get_supp_group() # Get the user's first supplementary group name, if any
{
	echo $(id $1 | sed -E '
		s/^uid=-?[0-9]+\([^)]+\) //;
		s/^gid=-?[0-9]+\([^)]+\) //;
		s/^groups=-?[0-9]+\([^)]+\)//;
		s/^,//;
		s/,.*$//;
		s/^-?[0-9]+\(//;
		s/\)$//
	')
}

daemon_group=$(get_group daemon)
nobody_group=$(get_group nobody)
nobody_supp_group=$(get_supp_group nobody)

# Run normal test as setuid daemon (to test AT_EACCESS)

echo setuid daemon
setup daemon $daemon_group 4755
check sudo -u nobody $s
clean

# Run normal test as setuid root (to test AT_EACCESS)

echo setuid root
setup root wheel 4755
check sudo -u nobody $s
clean

# Test different numbers of leading dirs and leading dirs with different
# permissions (to test leading executable check)

setup daemon $daemon_group 4755
setid daemon $daemon_group 644 $t/f touch
setid daemon $daemon_group 755 $t/d1 mkdir
setid daemon $daemon_group 000 $t/d2 mkdir
setid daemon $daemon_group 644 $t/d1/f touch
setid daemon $daemon_group 644 $t/d2/f touch

echo leading dirs ruid=nobody euid=daemon
check sudo -u nobody $s test test/ \
	$t $t/ $t/f \
	$t/d1 $t/d1/ $t/d1/f \
	$t/d2 $t/d2/ $t/d2/f

echo leading dirs ruid=root euid=daemon
check $s test test/ \
	$t $t/ $t/f \
	$t/d1 $t/d1/ $t/d1/f \
	$t/d2 $t/d2/ $t/d2/f

chmod 755 $s

echo leading dirs nobody
check sudo -u nobody $s test test/ \
	$t $t/ $t/f \
	$t/d1 $t/d1/ $t/d1/f \
	$t/d2 $t/d2/ $t/d2/f

echo leading dirs root
check $s test test/ \
	$t $t/ $t/f \
	$t/d1 $t/d1/ $t/d1/f \
	$t/d2 $t/d2/ $t/d2/f

clean

# Test lots of permissions without setuid/setgid with the same user's files
# (to test when uid matches)

modes="444 400 040 004 222 200 020 002 111 100 010 001 000 755 644 777 4755 2755 1777"

checkperms() # usage: checkperms fuser fgroup mode
{
	setup nobody $nobody_group $3
	cases=
	for m in $modes
	do
		setid $1 $2 $m $t/$m touch
		setid $1 $2 $m $t/$m.d mkdir
		cases="$cases $t/$m $t/$m.d"
	done
	sudo -u nobody $s $cases
	clean
}

echo perm same user
checkperms nobody $nobody_group 755

# Test lots of permissions without setuid/setgid with different user's files
# (to test when uid/gid don't match)

echo perm diff user
checkperms daemon $daemon_group 755

# Test lots of permissions without setuid/setgid with different user's files
# but the same group (to test when uid doesn't match but gid does match)

echo perm same group
checkperms daemon $nobody_group 755

# Test lots of permissions without setuid/setgid with different user's files
# but same supplementary group (to test when uid/gid don't match but a
# supplementary group matches)

if [ -n "$nobody_supp_group" ]
then
	echo perm same supp group
	checkperms daemon $nobody_supp_group 755
fi

# Test lots of permissions with setuid with different user's files
# (to test setuid)

echo perm setuid daemon
checkperms daemon $daemon_group 4755

# Test lots of permissions with setgid with different user's files
# (to test setgid)

echo perm setgid daemon
checkperms daemon $daemon_group 2755

exit $fail

# vi:set ts=4 sw=4:
