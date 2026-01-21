#include "cfl_test.h"
#include "cfl_lock.h"
#include "cfl_thread.h"
#include "cfl_atomic.h"

static CFL_LOCKP lock;
static int shared_resource = 0;

static void lock_thread(void *arg) {
    for (int i = 0; i < 1000; i++) {
        cfl_lock_acquire(lock);
        shared_resource++;
        cfl_lock_release(lock);
    }
}

TEST_CASE(test_cfl_lock_mutex) {
    lock = cfl_lock_new();
    shared_resource = 0;

    CFL_THREADP t1 = cfl_thread_new(lock_thread);
    CFL_THREADP t2 = cfl_thread_new(lock_thread);

    cfl_thread_start(t1, NULL);
    cfl_thread_start(t2, NULL);

    cfl_thread_wait(t1);
    cfl_thread_wait(t2);

    cfl_thread_free(t1);
    cfl_thread_free(t2);

    TEST_ASSERT_EQUAL_INT(2000, shared_resource);

    cfl_lock_free(lock);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_lock_mutex);
TEST_SUITE_END()
