/*
 * Copyright (c) 2000-2003 Apple Computer, Inc. All rights reserved.
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
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */

/*
 * This is an excerpt from pthread.h containing the portion that needs
 * to be reincluded in the relevant 10.4 _DARWIN_C_SOURCE case.
 */

#ifndef _MACH_PORT_T
#define _MACH_PORT_T
typedef __darwin_mach_port_t		mach_port_t;
#endif

#ifndef _SIGSET_T
#define _SIGSET_T
typedef __darwin_sigset_t		sigset_t;
#endif

__BEGIN_DECLS

/* returns non-zero if pthread_create or cthread_fork have been called */
int		pthread_is_threaded_np(void);

/* returns non-zero if the current thread is the main thread */
int		pthread_main_np(void);

/* return the mach thread bound to the pthread */
mach_port_t 	pthread_mach_thread_np(pthread_t);
size_t	 	pthread_get_stacksize_np(pthread_t);
void *		pthread_get_stackaddr_np(pthread_t);

/* Like pthread_cond_signal(), but only wake up the specified pthread */
int		pthread_cond_signal_thread_np(pthread_cond_t *, pthread_t);

/* Like pthread_cond_timedwait, but use a relative timeout */
int		pthread_cond_timedwait_relative_np(pthread_cond_t *cond, 
				 pthread_mutex_t *mutex,
				 const struct timespec *reltime);

/* Like pthread_create(), but leaves the thread suspended */
int       pthread_create_suspended_np(pthread_t *thread, 
                         const pthread_attr_t *attr,
                         void *(*start_routine)(void *), 
                         void *arg);
int       pthread_kill(pthread_t, int);
int       pthread_sigmask(int, const sigset_t *, sigset_t *);
void	  pthread_yield_np(void);

__END_DECLS
