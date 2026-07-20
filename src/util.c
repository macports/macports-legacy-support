/*
 * Copyright (c) 2025 Frederick H. G. Wright II <fw@fwright.net>
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

#include "rosetta.h"
#include "util.h"

#if __MPLS_NEED_CHECK_ACCESS__

#include <stdint.h>
#include <unistd.h>

#include <mach/mach.h>
#include <mach/mach_vm.h>

#include "compiler.h"

/* ptr->int casts require matching sizes */
#ifndef __LP64__
typedef uint32_t adrint_t;
#else
typedef uint64_t adrint_t;
#endif

/*
 * Check a given address and size for validity and needed access.
 *
 * Unfortunately there's no straightforward call for this, so it has
 * to resort to checking for a compatible memory region, and then
 * iterating as needed for any additional range.
 *
 * If the okadr arg is not NULL, then it represents a known valid address.
 * If the range to be checked lies within the same page, we can skip the
 * OS validation.
 */
int
__mpls_check_access(void *adr, mach_vm_size_t size, vm_prot_t access,
                    void *okadr)
{
  vm_map_t task = mach_task_self();
  mach_vm_address_t address;
  mach_vm_size_t msize;
  vm_region_basic_info_data_64_t info;
  mach_msg_type_number_t count;
  mach_port_t object_name;
  kern_return_t ret;
  adrint_t start_adr = (adrint_t) adr;
  adrint_t end_adr = start_adr + size;
  adrint_t okpage;
  static adrint_t pagemask = 0;

  if (okadr) {
    if (MPLS_SLOWPATH(!pagemask)) {
      pagemask = getpagesize();
      if (pagemask) --pagemask;
    }

    okpage = ((adrint_t) okadr) & ~pagemask;
    if ((start_adr & ~pagemask) == okpage
        && ((end_adr - 1) & ~pagemask) == okpage) return 0;
  }

  address = start_adr;
  msize = 0;
  count = VM_REGION_BASIC_INFO_COUNT_64;
  ret = mach_vm_region(task, &address, &msize, VM_REGION_BASIC_INFO_64,
                       (vm_region_info_t)&info, &count, &object_name);
  if (ret != KERN_SUCCESS) return -1;
  /*
   * If the first valid region on or after our address is later, then
   * our address is invalid.
   */
  if (start_adr < address) return -1;
  if (access & ~info.protection) return -1;

  /* Verify that we have contiguous valid regions covering our range. */
  while (end_adr > (address += msize)) {
    msize = 0;
    count = VM_REGION_BASIC_INFO_COUNT_64;
    ret = mach_vm_region(task, &address, &msize, VM_REGION_BASIC_INFO_64,
                         (vm_region_info_t)&info, &count, &object_name);
    if (ret != KERN_SUCCESS) return -1;
    if (end_adr <= address) return -1;
    if (access & ~info.protection) return -1;
  }

  return 0;
}

#endif /* __MPLS_NEED_CHECK_ACCESS__ */

#if __MPLS_LIB_ROSETTA1_HANDLING__

#include <dlfcn.h>

#include <sys/sysctl.h>
#include <sys/types.h>

uint64_t __mpls_rosetta1_bugs = 0;

/* Determine whether we're running under Rosetta 1, and which bugs apply */
static void
setup_rosetta1(void)
{
  int native;
  size_t native_sz = sizeof(native);

  if (sysctlbyname("sysctl.proc_native", &native, &native_sz, NULL, 0) < 0) {
    /* If sysctl failed, must be real ppc. */
    __mpls_is_rosetta = 0;
  } else {
    __mpls_is_rosetta = native ? 0 : 1;
  }
  if (!__mpls_is_rosetta) return;

  __mpls_rosetta1_bugs = _ROSETTA1_BUGS_ALL;

  /* Use existence of pthread_from_mach_thread_np() as proxy for 10.5+ */
  if (dlsym(RTLD_NEXT, "pthread_from_mach_thread_np")) {
    __mpls_rosetta1_bugs &= ~((uint64_t) _ROSETTA1_BUGS_TIGER);
  }
}

#else  /* !__MPLS_LIB_ROSETTA1_HANDLING__ */

#define setup_rosetta1(x)

#endif  /* !__MPLS_LIB_ROSETTA1_HANDLING__ */

#if __MPLS_LIB_ROSETTA2_HANDLING__

#include <sys/sysctl.h>
#include <sys/types.h>

/* Determine whether we're running under Rosetta 2, and which bugs apply */
static void
setup_rosetta2(void)
{
  int translated;
  size_t translated_sz = sizeof(translated);

  if (sysctlbyname("sysctl.proc_translated", &translated, &translated_sz,
                   NULL, 0) < 0) {
    /* If sysctl failed, must be really native. */
    __mpls_is_rosetta = 0;
  } else {
    __mpls_is_rosetta = translated ? 2 : 0;
  }
  if (!__mpls_is_rosetta) return;

  __mpls_rosetta2_bugs = _ROSETTA2_BUGS_ALL;
}

#else  /* !__MPLS_LIB_ROSETTA2_HANDLING__ */

#define setup_rosetta2(x)

#endif  /* !__MPLS_LIB_ROSETTA2_HANDLING__ */

#if __MPLS_LIB_ROSETTA1_HANDLING__ || __MPLS_LIB_ROSETTA2_HANDLING__

/* -1 = uninit, 0 = native, 1 = Rosetta 1, 2 = Rosetta 2 */
int __mpls_is_rosetta = -1;

/* Do Rosetta setup as requested */
void
__mpls_setup_rosetta(void)
{
  if (__mpls_is_rosetta >= 0) return;
  setup_rosetta1();
  setup_rosetta2();
}

/* Do Rosetta setup at program launch */
static void __attribute__((constructor))
init_rosetta(void)
{
  __mpls_setup_rosetta();
}

#endif  /* __MPLS_LIB_ROSETTA1_HANDLING__ || __MPLS_LIB_ROSETTA2_HANDLING__ */
