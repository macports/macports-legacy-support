/*
 * Copyright (c) 2022
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

#ifndef _MACPORTS_COPYFILE_H_
#define _MACPORTS_COPYFILE_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_SDK_MAJOR < 1050

#include <_macports_extras/tiger_only/copyfile.h>

/* The replacement copyfile.h doesn't need the wrapper */

#else /* __MPLS_SDK_MAJOR >= 1050 */

/* Include the primary system copyfile.h */
#include_next <copyfile.h>

#endif /* __MPLS_SDK_MAJOR >= 1050 */

#if __MPLS_SDK_SUPPORT_COPYFILE_WRAP__

#define COPYFILE_STATE_STATUS_CB        6
#define COPYFILE_STATE_STATUS_CTX       7
#define COPYFILE_STATE_COPIED           8

#endif /* __MPLS_SDK_SUPPORT_COPYFILE_WRAP__ */

#endif /* _MACPORTS_COPYFILE_H_ */
