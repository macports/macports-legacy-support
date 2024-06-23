/*
 * Copyright (c) 2020
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

#ifndef _MACPORTS_NETIF_H_
#define _MACPORTS_NETIF_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

/*
 * This header is automatically included by <net/if.h> on systems 10.9 and up.
 * It is therefore expected to be included by most current software.
 */
#if __MPLS_SDK_NETIF_SOCKET_FIX__
#  include <sys/socket.h>
#endif /* __MPLS_SDK_NETIF_SOCKET_FIX__ */

/* Include the primary system <net/if.h> */
#include_next <net/if.h>

#endif /* _MACPORTS_NETIF_H_ */
