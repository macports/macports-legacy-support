#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

size_t stack_size_main = 0;
size_t stack_size_thread = 0;
size_t stack_size_detached = 0;

void *thread_function(void *arg) {
     assert( !pthread_main_np() && "Unknown error, thread should *not* be main" );
     stack_size_thread = pthread_get_stacksize_np(pthread_self());
     assert( stack_size_thread && "Unknown error, stack size should *not* be zero" );
     return NULL;
}

void *detached_function(void *arg) {
     assert( !pthread_main_np() && "Unknown error, thread should *not* be main" );
     stack_size_detached = pthread_get_stacksize_np(pthread_self());
     assert( stack_size_detached && "Unknown error, stack size should *not* be zero" );
     return NULL;
}

int main()
{
     pthread_t thread;
     pthread_t thread_detached;
     pthread_attr_t attr;

     assert( pthread_main_np() && "Unknown error, thread should be main." );
     stack_size_main = pthread_get_stacksize_np(pthread_self());

     assert( !pthread_create(&thread, NULL, thread_function, NULL) && "Unknown error, cannot create first thread" );
     pthread_join(thread, NULL);

     assert( !pthread_attr_init(&attr) && "Unknown error, cannot initialises attribute" );
     assert( !pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) && "Unknown error, cannot set detached state" );
     assert( !pthread_create(&thread_detached, &attr, detached_function, NULL) && "Unknown error, cannot create second thread" );
     assert( !pthread_attr_destroy(&attr) && "Unknown error, cannot destroy attribute" );

     do {
          usleep(1000);
     } while( !stack_size_detached );

     assert( stack_size_thread == stack_size_detached && "Unknown Error, non-main threads are assumed to have the same stack size" );
     assert( stack_size_main > stack_size_thread && "Default stack size for main thread is the same as the default stack size of non-main threads" );

     printf("Stack size for the non-main thread (%zu) is larger than for non-main threads (%zu), which is expected.\n", stack_size_main, stack_size_thread);

     return 0;
}
