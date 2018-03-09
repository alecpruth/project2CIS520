/* Tests timer_sleep(-100).  Only requirement is that it not crash. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

void thread1();
void thread2();
void thread3();

struct lock test_lock;


void
test_my_alarm (void) 
{
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  pass ();
}

void test_my_threads (void)
{
    thread_set_priority(PRI_MAX);
    
    lock_init(&test_lock);
    
    thread_create ("Thread 1", PRI_DEFAULT, thread1, NULL);
    timer_sleep(10); // main thread goes to sleep
    
    thread_create ("Thread 2", PRI_DEFAULT+1, thread2, NULL);
    timer_sleep(10);  // main thread goes to sleep 
    
    thread_create ("Thread 3", PRI_MAX, thread3, NULL);
    
    printf("All threads created! Main thread now exiting...\n");
    return;
}

void thread1()
{
static uint8_t once = 1;

    while(1) {
        printf("Thread 1 executing...\n");
        lock_acquire(&test_lock);
        printf("Thread 1 acquired the lock!\n");
        if(once) {
            printf("Thread 1 going to sleep zzzzzZ\n");
            timer_sleep(100);
            once = 0;
        }
        printf("Thread 1 trying to release the lock ...\n");
        lock_release(&test_lock);
        printf("Thread 1 released the lock!\n");
    }
}


void thread2()
{
    while(1) {
        printf("Thread 2 executing...\n");
    }
}

void thread3()
{
    while(1) {
        printf("Thread 3 executing...\n");
        lock_acquire(&test_lock);
        printf("Thread 3 acquired the lock!\n");
        lock_release(&test_lock);
        printf("Thread 3 released the lock and now exiting...\n");
        break;
    }
}
