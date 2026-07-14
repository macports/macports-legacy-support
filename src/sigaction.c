/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved
 *
 *	@(#)sigaction.c	1.0
 */

/*
 * NOTICE: This file was modified in June 2026 to allow
 * for use as a supporting file for MacPorts legacy support library.
 * This notice is included in support of clause 2.2 (b) of the
 * Apple Public License, Version 2.0.
 *
 * The original file is taken from the Apple public sources at:
 * https://github.com/apple-oss-distributions/Libc/blob/33cf694a25b02c895c450328ba8294c8200ef077/sys/sigaction.c
 *
 * Changes include:
 *   Making the build conditional on the need for its use.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_FIX_PPC64_SIGNALS__

/*
 * The remainder of this file (except for the final #endif) is taken verbatim
 * from the original source.  It needs to be duplicated as an overlay to
 * use the corrected _sigtramp() implementation.  Since the bug in question
 * does not appear to affect signal() or bsd_signal(), those functions
 * don't require this treatment.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include <errno.h>

/*
 *	Intercept the sigaction syscall and use our signal trampoline
 *	as the signal handler instead.  The code here is derived
 *	from sigvec in sys/kern_sig.c.
 */

int
sigaction (int sig, const struct sigaction * __restrict nsv, struct sigaction * __restrict osv)
{
	extern void _sigtramp();
	struct __sigaction sa;
	struct __sigaction *sap;

	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP) {
	        errno = EINVAL;
	        return (-1);
	}
	sap = (struct __sigaction *)0;
	if (nsv) {
		sa.sa_handler = nsv->sa_handler;
		sa.sa_tramp = _sigtramp;
		sa.sa_mask = nsv->sa_mask;
		sa.sa_flags = nsv->sa_flags;	
		sap = &sa;
	}
	if (syscall (SYS_sigaction, sig, sap, osv) < 0) {
	        return (-1);
	}
	return (0);
}

// XXX
#ifdef __DYNAMIC__

int
_sigaction_nobind (sig, nsv, osv)
        int sig;
	register const struct sigaction *nsv;
        register struct sigaction *osv;
{
    return sigaction(sig, nsv, osv);
}
#endif

#endif  /* __MPLS_LIB_FIX_PPC64_SIGNALS__ */

#if __MPLS_EMP_FIX_PPC64_SIGNALS__ && defined(__MPLS_UNIVERSAL__)

/* Avoid "no symbols" warning from ranlib */
void __mpls_empty_sigaction(void) {};

#endif  /* !empty universal slice */
