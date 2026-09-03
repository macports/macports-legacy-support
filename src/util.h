/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
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

/* This (conditionally) contains miscellaneous global utility features. */

/* MP support header */
#include "MacportsLegacySupport.h"

#include <dlfcn.h>
#include <stdlib.h>

#include "compiler.h"

/* Composite conditionals, determining need for functions */

#define __MPLS_NEED_CHECK_ACCESS__ \
    (__MPLS_LIB_FIX_TIGER_PPC64__ \
     || __MPLS_LIB_SUPPORT_STAT64__)

/*
 * Macro to abort() while saving reason string.
 *
 * This provides a way to record the reason for the abort() without relying
 * on any form of I/O, by simply saving a pointer to a reason string, for
 * possible examination by a debugger.  The global variable holding the message
 * is declared 'common', so that it can be declared in each module that uses it
 * without a conflict, and if no currently built module uses it, it won't exist.
 *
 * Unfortunately, it seems that declaring 'common' variables only works at
 * the top level, so the declaration can't be directly incorporated into
 * any macro that uses it; hence a separate macro is provided for the
 * declaration.  This macro must be invoked once in each module that uses
 * MPLS_ABORT, either directly or indirectly, paying attention to the
 * conditional structure.
 */

#define MPLS_ABORT(msg) { \
  __mpls_abortmsg = msg; \
  abort(); \
  }

#define DEFINE_MPLS_ABORTMSG char *__mpls_abortmsg __attribute__((common));

/*
 * Obtain the address of an OS function, with an optional suffix
 *
 * This provides both the variable and the code to obtain a pointer to
 * a given OS function via dlsym(), with an optional variant-related
 * suffix (e.g. '$UNIX2003') to use in the lookup.
 *
 * Args are:
 *   name: the standard function name
 *   suffix: the optional suffix
 */
#define GET_OS_ALT_FUNC(name, suffix) \
  static __typeof__(name) *os_##name = NULL; \
  \
  if (MPLS_SLOWPATH(!os_##name)) { \
    if (!(os_##name = dlsym(RTLD_NEXT, #name #suffix))) \
      MPLS_ABORT("lookup failed for _" #name #suffix) \
  }

/* Obtain the address of an OS function, without an optional suffix */
#define GET_OS_FUNC(name) GET_OS_ALT_FUNC(name,)

/* Version which makes the suffix runtime-optional */
#define GET_OS_OPT_ALT_FUNC(name, suffix) \
  static __typeof__(name) *os_##name = NULL; \
  \
  if (MPLS_SLOWPATH(!os_##name)) { \
    if (!(os_##name = dlsym(RTLD_NEXT, #name #suffix)) \
        && !(os_##name = dlsym(RTLD_NEXT, #name))) \
      MPLS_ABORT("lookup failed for _" #name #suffix "and _" #name) \
  }

#if __MPLS_NEED_CHECK_ACCESS__

#include <mach/mach_vm.h>

int __mpls_check_access(void *adr, mach_vm_size_t size, vm_prot_t access,
                        void *okadr);

#endif /* __MPLS_NEED_CHECK_ACCESS__ */
