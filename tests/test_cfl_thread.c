#include "cfl_test.h"
#include "cfl_thread.h"
#include "cfl_atomic.h"
#include "cfl_os.h"

static CFL_INT32 global_counter = 0;

static void thread_func(void *arg) {
    CFL_INT32 *counter = (CFL_INT32 *)arg;
    cfl_atomic_addInt32(counter, 1);
}

TEST_CASE(test_cfl_thread_create_join) {
    global_counter = 0;

    CFL_THREADP t1 = cfl_thread_new(thread_func);
    t1->param = &global_counter; // Set param manually or via start? Start takes param.
    CFL_THREADP t2 = cfl_thread_new(thread_func);

    // cfl_thread_new takes func. cfl_thread_start takes param.
    cfl_thread_start(t1, &global_counter);
    cfl_thread_start(t2, &global_counter);

    TEST_ASSERT(t1 != NULL);
    TEST_ASSERT(t2 != NULL);

    cfl_thread_wait(t1);
    cfl_thread_wait(t2);

    cfl_thread_free(t1);
    cfl_thread_free(t2);

    TEST_ASSERT_EQUAL_INT(2, global_counter);
}

static void sleep_thread(void *arg) {
    cfl_thread_sleep(100);
    (*(int*)arg) = 1;
}

TEST_CASE(test_cfl_thread_sleep) {
    int done = 0;
    CFL_THREADP t = cfl_thread_new(sleep_thread);
    cfl_thread_start(t, &done);

    TEST_ASSERT_EQUAL_INT(0, done);
    cfl_thread_wait(t);
    TEST_ASSERT_EQUAL_INT(1, done);

    cfl_thread_free(t);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_thread_create_join);
    RUN_TEST(test_cfl_thread_sleep);
TEST_SUITE_END()
