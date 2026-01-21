#include "cfl_test.h"
#include "cfl_atomic.h"

TEST_CASE(test_cfl_atomic_int32) {
    CFL_INT32 var = 10;

    // Test Set
    cfl_atomic_setInt32(&var, 20);
    TEST_ASSERT_EQUAL_INT(20, var);

    // Test CompareAndSet (Success)
    CFL_INT32 old = cfl_atomic_compareAndSetInt32(&var, 20, 30);
    TEST_ASSERT_EQUAL_INT(20, old);
    TEST_ASSERT_EQUAL_INT(30, var);

    // Test CompareAndSet (Fail)
    old = cfl_atomic_compareAndSetInt32(&var, 999, 40); // Expected 999, but is 30
    TEST_ASSERT_EQUAL_INT(30, old); // Should return current value
    TEST_ASSERT_EQUAL_INT(30, var); // Should not change

    // Test Add
    old = cfl_atomic_addInt32(&var, 5);
    TEST_ASSERT_EQUAL_INT(30, old);
    TEST_ASSERT_EQUAL_INT(35, var);

    // Test Sub
    old = cfl_atomic_subInt32(&var, 5);
    TEST_ASSERT_EQUAL_INT(35, old);
    TEST_ASSERT_EQUAL_INT(30, var);
}

TEST_CASE(test_cfl_atomic_bool) {
    CFL_BOOL flag = CFL_FALSE;

    cfl_atomic_setBoolean(&flag, CFL_TRUE);
    TEST_ASSERT_EQUAL_INT(CFL_TRUE, flag);

    CFL_BOOL old = cfl_atomic_compareAndSetBoolean(&flag, CFL_TRUE, CFL_FALSE);
    TEST_ASSERT_EQUAL_INT(CFL_TRUE, old);
    TEST_ASSERT_EQUAL_INT(CFL_FALSE, flag);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_atomic_int32);
    RUN_TEST(test_cfl_atomic_bool);
TEST_SUITE_END()
