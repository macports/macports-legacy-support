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
