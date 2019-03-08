/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _LIBPROC_H_
#define _LIBPROC_H_

#include <sys/buf.h>
#include <sys/proc.h>
#include <mach/mach_types.h>
#include <sys/proc_info.h>

int proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize);
int proc_pidinfo(int pid, int flavor, uint64_t arg,  void *buffer, int buffersize);
int proc_pidfdinfo(int pid, int fd, int flavor, void * buffer, int buffersize);
int proc_name(int pid, void * buffer, uint32_t buffersize);
int proc_regionfilename(int pid, uint64_t address, void * buffer, uint32_t buffersize);
int proc_kmsgbuf(void * buffer, uint32_t buffersize);

#endif /*_LIBPROC_H_ */
