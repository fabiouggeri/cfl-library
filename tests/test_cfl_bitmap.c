#include "cfl_test.h"
#include "cfl_bitmap.h"

TEST_CASE(test_cfl_bitmap_new_free) {
    CFL_BITMAPP bm = cfl_bitmap_new(100);
    TEST_ASSERT(bm != NULL);
    TEST_ASSERT(bm->map != NULL);
    TEST_ASSERT_EQUAL_INT((100 / 8) + 1, bm->uiSize);
    cfl_bitmap_free(bm);
}

TEST_CASE(test_cfl_bitmap_set_get_reset) {
    CFL_BITMAPP bm = cfl_bitmap_new(10);

    TEST_ASSERT_EQUAL_INT(0, cfl_bitmap_get(bm, 5));

    cfl_bitmap_set(bm, 5);
    TEST_ASSERT_EQUAL_INT(1, cfl_bitmap_get(bm, 5));

    cfl_bitmap_reset(bm, 5);
    TEST_ASSERT_EQUAL_INT(0, cfl_bitmap_get(bm, 5));

    cfl_bitmap_free(bm);
}

TEST_CASE(test_cfl_bitmap_clear) {
    CFL_BITMAPP bm = cfl_bitmap_new(16);
    cfl_bitmap_set(bm, 1);
    cfl_bitmap_set(bm, 15);

    cfl_bitmap_clear(bm);

    TEST_ASSERT_EQUAL_INT(0, cfl_bitmap_get(bm, 1));
    TEST_ASSERT_EQUAL_INT(0, cfl_bitmap_get(bm, 15));

    cfl_bitmap_free(bm);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_bitmap_new_free);
    RUN_TEST(test_cfl_bitmap_set_get_reset);
    RUN_TEST(test_cfl_bitmap_clear);
TEST_SUITE_END()
