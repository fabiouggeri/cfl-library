#include "cfl_test.h"
#include "cfl_array.h"

TEST_CASE(test_cfl_array_new_free) {
    CFL_ARRAYP array = cfl_array_new(10, sizeof(int));
    TEST_ASSERT(array != NULL);
    TEST_ASSERT(array->items != NULL);
    TEST_ASSERT_EQUAL_INT(0, array->ulLength);
    TEST_ASSERT(array->ulCapacity >= 10);
    TEST_ASSERT_EQUAL_INT(sizeof(int), array->ulItemSize);
    cfl_array_free(array);
}

TEST_CASE(test_cfl_array_add_get) {
    CFL_ARRAYP array = cfl_array_new(5, sizeof(int));

    int val1 = 10, val2 = 20, val3 = 30;
    *(int *)cfl_array_add(array) = val1;
    *(int *)cfl_array_add(array) = val2;

    TEST_ASSERT_EQUAL_INT(2, array->ulLength);

    int *ret1 = (int *)cfl_array_get(array, 0);
    int *ret2 = (int *)cfl_array_get(array, 1);

    TEST_ASSERT(ret1 != NULL);
    TEST_ASSERT(ret2 != NULL);
    TEST_ASSERT_EQUAL_INT(10, *ret1);
    TEST_ASSERT_EQUAL_INT(20, *ret2);

    // Add causing resize (if we started small, but 5 is enough here)
    *(int *)cfl_array_add(array) = val3;
    int *ret3 = (int *)cfl_array_get(array, 2);
    TEST_ASSERT_EQUAL_INT(30, *ret3);

    cfl_array_free(array);
}

TEST_CASE(test_cfl_array_remove) {
    CFL_ARRAYP array = cfl_array_new(5, sizeof(int));
    int nums[] = {1, 2, 3};

    *(int *)cfl_array_add(array) = nums[0];
    *(int *)cfl_array_add(array) = nums[1];
    *(int *)cfl_array_add(array) = nums[2];

    TEST_ASSERT_EQUAL_INT(3, cfl_array_length(array));

    cfl_array_del(array, 1); // Remove '2'

    TEST_ASSERT_EQUAL_INT(2, cfl_array_length(array));
    TEST_ASSERT_EQUAL_INT(1, *(int*)cfl_array_get(array, 0));
    TEST_ASSERT_EQUAL_INT(3, *(int*)cfl_array_get(array, 1));

    cfl_array_free(array);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_array_new_free);
    RUN_TEST(test_cfl_array_add_get);
    RUN_TEST(test_cfl_array_remove);
TEST_SUITE_END()
