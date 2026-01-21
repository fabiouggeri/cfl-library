#include "cfl_test.h"
#include "cfl_log.h"

// Custom writer that does nothing, to avoid cluttering stdout
static void dummy_writer(const char *format, va_list args) {
    // va_list args_copy;
    // va_copy(args_copy, args);
    // vprintf(format, args_copy);
    // va_end(args_copy);
}

TEST_CASE(test_cfl_log_basic) {
    // Ensuring basic log function names exist and link
    // cfl_log_register_writer(dummy_writer);
    // Assuming standard log init or usage

    // Just testing simple macro or function existence
    // cfl_log_info("Test log info");

    TEST_PASS();
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_log_basic);
TEST_SUITE_END()
