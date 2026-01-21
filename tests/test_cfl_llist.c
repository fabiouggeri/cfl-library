#include "cfl_test.h"
#include "cfl_llist.h"

TEST_CASE(test_cfl_llist_lifecycle) {
    CFL_LLISTP list = cfl_llist_new(0);
    TEST_ASSERT(list != NULL);
    cfl_llist_free(list);
}

TEST_CASE(test_cfl_llist_add_remove) {
    CFL_LLISTP list = cfl_llist_new(0);

    char v1[] = "A";
    char v2[] = "B";
    char v3[] = "C";

    cfl_llist_addLast(list, v1);
    cfl_llist_addLast(list, v2);
    cfl_llist_addFirst(list, v3); // Should allow C, A, B order

    // Check First/Last
    TEST_ASSERT_EQUAL_STRING("C", (char*)cfl_llist_getFirst(list));
    TEST_ASSERT_EQUAL_STRING("B", (char*)cfl_llist_getLast(list));

    // Remove First
    char *r1 = (char*)cfl_llist_removeFirst(list);
    TEST_ASSERT_EQUAL_STRING("C", r1);

    // Remove Last
    char *r2 = (char*)cfl_llist_removeLast(list);
    TEST_ASSERT_EQUAL_STRING("B", r2);

    // One left
    TEST_ASSERT_EQUAL_STRING("A", (char*)cfl_llist_getFirst(list));

    cfl_llist_free(list);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_llist_lifecycle);
    RUN_TEST(test_cfl_llist_add_remove);
TEST_SUITE_END()
