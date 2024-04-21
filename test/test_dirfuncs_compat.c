/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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

/*
 * This leverages the existing tests for the old __mpls_* wrappers as tests
 * for the new transparent wrappers provided for compatibility, by defining
 * the relevant functions as macros and reusing all of test_fdopendir.
 *
 * For OS versions that never used the wrappers, this entire test is a dummy.
 */

/* MP support header */
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_FDOPENDIR__

#include "../src/dirfuncs_compat.h"

#define opendir     __mpls_opendir
#define readdir     __mpls_readdir
#define readdir_r   __mpls_readdir_r
#define telldir     __mpls_telldir
#define seekdir     __mpls_seekdir
#define rewinddir   __mpls_rewinddir
#define closedir    __mpls_closedir
#undef dirfd
#define dirfd       __mpls_dirfd

#include "test_fdopendir.c"

#else /* !__MP_LEGACY_SUPPORT_FDOPENDIR__ */

int main(){ return 0; }

#endif /* !__MP_LEGACY_SUPPORT_FDOPENDIR__ */
