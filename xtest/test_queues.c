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
 * This provides tests for some functions from sys/queue.h., currently:
 *    STAILQ_FOREACH
 *    SLIST_REMOVE_AFTER
 */

#include <assert.h>
#include <stddef.h>  /* For NULL */
#include <sys/queue.h>


/* Tests for SLIST_REMOVE_AFTER */

#ifndef SLIST_REMOVE_AFTER
#error SLIST_REMOVE_AFTER is undefined
#endif

#ifndef SLIST_HEAD_INITIALIZER
#error SLIST_HEAD_INITIALIZER is undefined
#endif

typedef struct slist_entry_s {
  int value;
  SLIST_ENTRY(slist_entry_s) next;
} slist_entry_t;

typedef SLIST_HEAD(slist_head_s, slist_entry_s) slist_head_t;

static slist_head_t slist_head = SLIST_HEAD_INITIALIZER(slist_head);

/* Some sample entries */
static slist_entry_t slist_entries[] = {
  {.value = 1},
  {.value = 2},
  {.value = 3},
};
#define SLIST_NUM (sizeof(slist_entries) / sizeof(slist_entries[0]))

static void
test_slist(void)
{
  slist_entry_t *ep, *tp;

  /* Fill the queue with the sample entries (reversed) */
  for (ep = &slist_entries[0]; ep < &slist_entries[SLIST_NUM]; ++ep) {
    SLIST_INSERT_HEAD(&slist_head, ep, next);
  }

  /* Get and check the first entry */
  ep = &slist_entries[SLIST_NUM - 1];
  tp = SLIST_FIRST(&slist_head);
  assert(tp->value == ep->value);

  /* Remove the following entry, then check the one after */
  SLIST_REMOVE_AFTER(tp, next);
  ep = &slist_entries[SLIST_NUM - 1 - 2];
  tp = SLIST_NEXT(tp, next);
  assert(tp->value == ep->value);
}


/* Tests for STAILQ_FOREACH */

#ifndef STAILQ_FOREACH
#error STAILQ_FOREACH is undefined
#endif

typedef struct stailq_entry_s {
  int value;
  STAILQ_ENTRY(stailq_entry_s) next;
} stailq_entry_t;

typedef STAILQ_HEAD(stailq_head_s, stailq_entry_s) stailq_head_t;

static stailq_head_t stailq_head = STAILQ_HEAD_INITIALIZER(stailq_head);

/* Some sample entries */
static stailq_entry_t stailq_entries[] = {
  {.value = 1},
  {.value = 2},
  {.value = 3},
};
#define STAILQ_NUM (sizeof(stailq_entries) / sizeof(stailq_entries[0]))

static void
test_stailq(void)
{
  stailq_entry_t *ep, *tp;

  /* Fill the queue with the sample entries */
  for (ep = &stailq_entries[0]; ep < &stailq_entries[STAILQ_NUM]; ++ep) {
    STAILQ_INSERT_TAIL(&stailq_head, ep, next);
  }

  /* See if STAILQ_FOREACH returns expected sequence */
  ep = &stailq_entries[0];
  STAILQ_FOREACH(tp, &stailq_head, next) {
    assert(tp->value == ep++->value);
  }

  /* Check expected end */
  assert(ep == &stailq_entries[STAILQ_NUM]);
}


int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  test_slist();
  test_stailq();

  return 0;
}
