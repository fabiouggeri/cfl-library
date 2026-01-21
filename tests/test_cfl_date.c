#include "cfl_test.h"
#include "cfl_date.h"

TEST_CASE(test_cfl_date_creation) {
    CFL_DATE date;
    cfl_date_getCurrent(&date);
    TEST_ASSERT(date.year > 2020);
    TEST_ASSERT(date.month >= 1 && date.month <= 12);
    TEST_ASSERT(date.day >= 1 && date.day <= 31);
}

TEST_CASE(test_cfl_date_manipulation) {
    CFL_DATE d1, d2;
    // Setup a specific date: 2023-01-01 12:00:00
    d1.year = 2023; d1.month = 1; d1.day = 1;
    d1.hour = 12; d1.min = 0; d1.sec = 0; d1.millis = 0;

    // Copy
    cfl_date_copy(&d1, &d2);
    TEST_ASSERT_EQUAL_INT(2023, d2.year);
    TEST_ASSERT_EQUAL_INT(1, d2.month);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_date_creation);
    RUN_TEST(test_cfl_date_manipulation);
TEST_SUITE_END()
