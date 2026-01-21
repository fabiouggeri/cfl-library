#include "cfl_test.h"
#include "cfl_os.h"
#include "cfl_thread.h"

TEST_CASE(test_cfl_os_detection) {
#if defined(CFL_OS_WINDOWS)
    printf("  [INFO] Detected OS: Windows\n");
#elif defined(CFL_OS_LINUX)
    printf("  [INFO] Detected OS: Linux\n");
#else
    printf("  [WARN] Unknown OS\n");
#endif

    // Test sleep (just ensure it doesn't crash)
    cfl_thread_sleep(10);
    TEST_PASS();
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_os_detection);
TEST_SUITE_END()
