#include "cfl_test.h"
#include "cfl_process.h"

TEST_CASE(test_cfl_process_pid) {
    CFL_INT64 pid = cfl_process_getId();
    printf("  [INFO] Current PID: %lld\n", pid);
    TEST_ASSERT(pid > 0);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_process_pid);
TEST_SUITE_END()
