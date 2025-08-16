#include <os/lock.h>
#include <iostream>
#include <cassert>

// see https://github.com/macports/macports-legacy-support/commit/c39f56bd335ac0f8692a5c67a9e5f0630dbbed58#r106208628
// for reason test is in C++

int main() {
    std::cout << "***os_unfair_lock started\n";

    {
	os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
	os_unfair_lock_lock(&lock);

	assert( !os_unfair_lock_trylock(&lock) && "should return false if the lock was already locked" );

	// critical section here

	os_unfair_lock_unlock(&lock);
    }

    {
	os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
	assert( os_unfair_lock_trylock(&lock) && "should return true if the lock was successfully locked");

	assert( !os_unfair_lock_trylock(&lock) && "should return false if the lock was already locked" );

	// critical section here

	os_unfair_lock_unlock(&lock);
    }

    std::cout << "***os_unfair_lock completed\n\n";
    return 0;
}
