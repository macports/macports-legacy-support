/*
 * Copyright (C) 2024 raf <raf@raf.org>
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

/* Based on the first example in the getattrlist(2) manual entry */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/attr.h>
#include <sys/errno.h>
#include <unistd.h>
#include <sys/vnode.h>

typedef struct attrlist attrlist_t;

struct FInfoAttrBuf
{
    u_int32_t    length;
    fsobj_type_t objType;
    char         finderInfo[32];
};
typedef struct FInfoAttrBuf FInfoAttrBuf;

static int FInfoDemo(int fd, const char *type, const char *creator)
{
    int          err;
    attrlist_t   attrList;
    FInfoAttrBuf attrBuf;

    assert(strlen(type) == 4);
    assert(strlen(creator) == 4);

    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
    attrList.commonattr  = ATTR_CMN_OBJTYPE | ATTR_CMN_FNDRINFO;

    err = fgetattrlist(fd, &attrList, &attrBuf, sizeof(attrBuf), 0);
    if (err != 0)
    {
        err = errno;
    }

    if ((err == 0) && (attrBuf.objType != VREG))
    {
        fprintf(stderr, "Not a standard file.");
        err = EINVAL;
    }
    else
    {
        memcpy(&attrBuf.finderInfo[0], type,    4);
        memcpy(&attrBuf.finderInfo[4], creator, 4);

        attrList.commonattr = ATTR_CMN_FNDRINFO;
        err = fsetattrlist(
            fd,
            &attrList,
            attrBuf.finderInfo,
            sizeof(attrBuf.finderInfo),
            0
        );
    }

    return err;
}

/* Note: Not an exhaustive test. Just call fsetattrlist(). */

int main(int ac, char **av)
{
    const char *fname = "./.jnk.tmp";
    int fd = open(fname, O_CREAT | O_RDWR, S_IRWXU);
    int rc = (FInfoDemo(fd, "TYPE", "CRTR") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    unlink(fname);
    if (rc != EXIT_SUCCESS)
        printf("fgetattrlist() failed, possibly correctly due to an unexpected filesystem type\n");
    return rc;
}

/* vi:set et ts=4 sw=4: */
