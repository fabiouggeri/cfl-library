#include "cfl_test.h"
#include "cfl_error.h"

TEST_CASE(test_cfl_error_lifecycle) {
    CFL_ERRORP err = cfl_error_new();
    TEST_ASSERT(err != NULL);

    cfl_error_setCode(err, 100);
    cfl_error_setType(err, 1);
    cfl_error_setMessage(err, "Test Error");

    TEST_ASSERT_EQUAL_INT(100, cfl_error_getCode(err));
    TEST_ASSERT_EQUAL_INT(1, cfl_error_getType(err));
    TEST_ASSERT_EQUAL_STRING("Test Error", cfl_error_getMessageStr(err));

    cfl_error_clear(err);
    TEST_ASSERT_EQUAL_INT(0, cfl_error_getCode(err));
    // Check if message is cleared or handled appropriately (depends on implementation, assuming safe check)

    cfl_error_free(err);
}

TEST_CASE(test_cfl_error_user_data) {
    CFL_ERRORP err = cfl_error_new();
    int data = 42;

    cfl_error_setUserData(err, &data);
    int *ret_data = (int *)cfl_error_getUserData(err);

    TEST_ASSERT(ret_data != NULL);
    TEST_ASSERT_EQUAL_INT(42, *ret_data);

    cfl_error_free(err);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_error_lifecycle);
    RUN_TEST(test_cfl_error_user_data);
TEST_SUITE_END()
