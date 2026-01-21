#include "cfl_test.h"
#include "cfl_event.h"
#include "cfl_thread.h"
#include "cfl_os.h"

static CFL_EVENTP event;

static void signaler_thread(void *arg) {
    cfl_thread_sleep(50);
    cfl_event_set(event);
}

TEST_CASE(test_cfl_event_wait) {
    event = cfl_event_new(NULL, CFL_FALSE); // Manual reset=false
    TEST_ASSERT(event != NULL);

    CFL_THREADP t = cfl_thread_new(signaler_thread);
    cfl_thread_start(t, NULL);

    // Wait for event
    CFL_BOOL result = cfl_event_wait(event);
    TEST_ASSERT(result == CFL_TRUE);

    cfl_thread_wait(t);
    cfl_thread_free(t);
    cfl_event_free(event);
}

TEST_CASE(test_cfl_event_timeout) {
    event = cfl_event_new(NULL, CFL_FALSE);

    CFL_BOOL result = cfl_event_waitTimeout(event, 10);
    TEST_ASSERT(result == CFL_FALSE);

    cfl_event_free(event);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_event_wait);
    RUN_TEST(test_cfl_event_timeout);
TEST_SUITE_END()
