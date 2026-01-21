#include "cfl_test.h"
#include "cfl_sync_queue.h"
#include "cfl_thread.h"
#include "cfl_os.h"

static void producer_thread(void *arg) {
    CFL_SYNC_QUEUEP queue = (CFL_SYNC_QUEUEP)arg;
    for (int i = 0; i < 5; i++) {
        cfl_sync_queue_putInt32(queue, i);
        cfl_thread_sleep(10);
    }
}

TEST_CASE(test_cfl_sync_queue_basic) {
    CFL_SYNC_QUEUEP queue = cfl_sync_queue_new(5);

    CFL_THREADP t = cfl_thread_new(producer_thread);
    cfl_thread_start(t, queue);

    for (int i = 0; i < 5; i++) {
        CFL_INT32 val = cfl_sync_queue_getInt32(queue);
        TEST_ASSERT_EQUAL_INT(i, val);
    }

    cfl_thread_wait(t);
    cfl_thread_free(t);
    cfl_sync_queue_free(queue);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_sync_queue_basic);
TEST_SUITE_END()
