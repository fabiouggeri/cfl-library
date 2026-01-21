#include "cfl_test.h"
#include "cfl_str.h"

TEST_CASE(test_cfl_str_new_free) {
    CFL_STRP str = cfl_str_new(10);
    TEST_ASSERT(str != NULL);
    TEST_ASSERT(str->data != NULL);
    TEST_ASSERT_EQUAL_INT(0, str->length);
    TEST_ASSERT(str->dataSize >= 10);
    cfl_str_free(str);
}

TEST_CASE(test_cfl_str_append) {
    CFL_STRP str = cfl_str_new(10);

    cfl_str_append(str, "Hello", NULL);
    TEST_ASSERT_EQUAL_STRING("Hello", str->data);
    TEST_ASSERT_EQUAL_INT(5, str->length);

    cfl_str_append(str, " World", NULL);
    TEST_ASSERT_EQUAL_STRING("Hello World", str->data);
    TEST_ASSERT_EQUAL_INT(11, str->length);

    cfl_str_free(str);
}

TEST_CASE(test_cfl_str_setFormat) {
    CFL_STRP str = cfl_str_new(10);

    cfl_str_setFormat(str, "Value: %d", 42);
    TEST_ASSERT_EQUAL_STRING("Value: 42", str->data);
    TEST_ASSERT_EQUAL_INT(9, str->length);

    cfl_str_free(str);
}

TEST_CASE(test_cfl_str_compare) {
    CFL_STRP str1 = cfl_str_new(10);
    CFL_STRP str2 = cfl_str_new(10);

    cfl_str_append(str1, "abc", NULL);
    cfl_str_append(str2, "abc", NULL);
    TEST_ASSERT(cfl_str_compare(str1, str2, CFL_TRUE) == 0);

    cfl_str_setFormat(str2, "abd");
    TEST_ASSERT(cfl_str_compare(str1, str2, CFL_TRUE) < 0);

    cfl_str_setFormat(str2, "abb");
    TEST_ASSERT(cfl_str_compare(str1, str2, CFL_TRUE) > 0);

    cfl_str_free(str1);
    cfl_str_free(str2);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_str_new_free);
    RUN_TEST(test_cfl_str_append);
    RUN_TEST(test_cfl_str_setFormat);
    RUN_TEST(test_cfl_str_compare);
TEST_SUITE_END()
