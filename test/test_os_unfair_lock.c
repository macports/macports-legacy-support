/* Test os_unfair_lock */

/*
 * This test was orginally in C++, with the following useless comment:
 *
 * // see https://github.com/macports/macports-legacy-support/commit/c39f56bd335ac0f8692a5c67a9e5f0630dbbed58#r106208628
 * // for reason test is in C++
 *
 * Neither the referenced commit nor its referenced ticket offer any
 * explanation as to why the test needs to be in C++.  Additionally,
 * Apple has started making C++ more problematic with mismatched SDKs,
 * making it undesirable to include C++ tests in the normal automatic
 * test suite.  Hence, this test has been rewritten in C, with the original
 * C++ version retained as a manual test.
 */

#include <assert.h>
#include <stdio.h>

#include <os/lock.h>

int
main(int argc, char *argv[])
{
  (void) argc; (void) argv;

  printf("Test of os_unfair_lock started\n");

  {
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock_lock(&lock);

    assert(!os_unfair_lock_trylock(&lock)
           && "should return false if the lock was already locked" );

    /* critical section would be here */

    os_unfair_lock_unlock(&lock);
  }

  {
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    assert(os_unfair_lock_trylock(&lock)
           && "should return true if the lock was successfully locked");

    assert(!os_unfair_lock_trylock(&lock)
           && "should return false if the lock was already locked" );

    /* critical section would be here */

    os_unfair_lock_unlock(&lock);
  }

    printf("Test of os_unfair_lock completed\n");

    return 0;
}
