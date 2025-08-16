/*
 * Copyright (c) 2025
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

/* MP support header */
#include "MacportsLegacySupport.h"

/* Do our SDK-related setup */
#include <_macports_extras/sdkversion.h>

#if __MPLS_LIB_SUPPORT_OS_UNFAIR_LOCK__

/* Allow building with later SDK. */
#if __MPLS_SDK_SUPPORT_OS_UNFAIR_LOCK__

#include <os/lock.h>

#else /* !__MPLS_SDK_SUPPORT_OS_UNFAIR_LOCK__ */

#include    <stdint.h>
#include    <stdbool.h>

typedef int32_t OSSpinLock;
typedef OSSpinLock *os_unfair_lock_t;

bool OSSpinLockTry( volatile OSSpinLock *__lock );
void OSSpinLockLock( volatile OSSpinLock *__lock );
void OSSpinLockUnlock( volatile OSSpinLock *__lock );

#endif /* !__MPLS_SDK_SUPPORT_OS_UNFAIR_LOCK__ */

/*
 * Note that, depending on the SDK used, the caller's os_unfair_lock_t might be:
 *
 *     int32_t *
 * or:
 *     struct {int32_t} *
 *
 * But this doesn't affect the actual code.
 */

void os_unfair_lock_lock(os_unfair_lock_t lock) {
     OSSpinLockLock(lock);
}

bool os_unfair_lock_trylock(os_unfair_lock_t lock) {
     return OSSpinLockTry(lock);
}

void os_unfair_lock_unlock(os_unfair_lock_t lock) {
     OSSpinLockUnlock(lock);
}

#endif /* __MPLS_LIB_SUPPORT_OS_UNFAIR_LOCK__ */
