#include "cfl_test.h"
#include "cfl_list.h"

TEST_CASE(test_cfl_list_new_free) {
    CFL_LISTP list = cfl_list_new(10);
    TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->items != NULL);
    TEST_ASSERT_EQUAL_INT(0, list->length);
    cfl_list_free(list);
}

TEST_CASE(test_cfl_list_add_get) {
    CFL_LISTP list = cfl_list_new(5);

    char *v1 = "One";
    char *v2 = "Two";

    cfl_list_add(list, v1);
    cfl_list_add(list, v2);

    TEST_ASSERT_EQUAL_INT(2, cfl_list_length(list));
    TEST_ASSERT_EQUAL_STRING("One", (char*)cfl_list_get(list, 0));
    TEST_ASSERT_EQUAL_STRING("Two", (char*)cfl_list_get(list, 1));

    cfl_list_free(list);
}

TEST_CASE(test_cfl_list_del) {
    CFL_LISTP list = cfl_list_new(5);
    cfl_list_add(list, "A");
    cfl_list_add(list, "B");
    cfl_list_add(list, "C");

    cfl_list_del(list, 1); // remove B

    TEST_ASSERT_EQUAL_INT(2, cfl_list_length(list));
    TEST_ASSERT_EQUAL_STRING("A", (char*)cfl_list_get(list, 0));
    TEST_ASSERT_EQUAL_STRING("C", (char*)cfl_list_get(list, 1));

    cfl_list_free(list);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_list_new_free);
    RUN_TEST(test_cfl_list_add_get);
    RUN_TEST(test_cfl_list_del);
TEST_SUITE_END()
