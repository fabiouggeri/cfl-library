#include "cfl_test.h"
#include "cfl_mem.h"

TEST_CASE(test_cfl_malloc_free) {
    void *ptr = cfl_malloc(100);
    TEST_ASSERT(ptr != NULL);
    cfl_free(ptr);
}

TEST_CASE(test_cfl_calloc) {
    int *ptr = (int *)cfl_calloc(10, sizeof(int));
    TEST_ASSERT(ptr != NULL);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(0, ptr[i]);
    }
    cfl_free(ptr);
}

TEST_CASE(test_cfl_realloc) {
    int *ptr = (int *)cfl_malloc(5 * sizeof(int));
    TEST_ASSERT(ptr != NULL);

    ptr[0] = 10;
    ptr[4] = 50;

    int *new_ptr = (int *)cfl_realloc(ptr, 10 * sizeof(int));
    TEST_ASSERT(new_ptr != NULL);

    // Check if data is preserved
    TEST_ASSERT_EQUAL_INT(10, new_ptr[0]);
    TEST_ASSERT_EQUAL_INT(50, new_ptr[4]);

    cfl_free(new_ptr);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_malloc_free);
    RUN_TEST(test_cfl_calloc);
    RUN_TEST(test_cfl_realloc);
TEST_SUITE_END()
