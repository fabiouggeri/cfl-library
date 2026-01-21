#include "cfl_test.h"
#include "cfl_btree.h"

// Helper comparison for int keys
static CFL_INT16 compare_int_keys(void *k1, void *k2, CFL_BOOL bExact) {
    int i1 = *(int*)k1;
    int i2 = *(int*)k2;
    (void)bExact; // Unused
    return (CFL_INT16)(i1 - i2);
}

TEST_CASE(test_cfl_btree_lifecycle) {
    // Assuming 5 keys per node
    CFL_BTREEP tree = cfl_btree_new(5, compare_int_keys);
    TEST_ASSERT(tree != NULL);
    // cfl_btree_numNodes not found in header?
    // Header only shows walk, iterator, search. No count function?
    // Maybe verify !tree->pRoot or similar? But struct details opaque if possible.
    // If numNodes not available, skip check or check not null.
    // Assuming numNodes doesn't exist based on header view.
    // TEST_ASSERT_EQUAL_INT(0, cfl_btree_numNodes(tree));
    cfl_btree_free(tree, NULL);
}

TEST_CASE(test_cfl_btree_add_find) {
    CFL_BTREEP tree = cfl_btree_new(3, compare_int_keys);

    int k1 = 10;
    int k2 = 5;
    int k3 = 20;
    int k4 = 15;

    cfl_btree_add(tree, &k1);
    cfl_btree_add(tree, &k2);
    cfl_btree_add(tree, &k3);
    cfl_btree_add(tree, &k4);

    // TEST_ASSERT_EQUAL_INT(4, cfl_btree_numNodes(tree));

    int *val = (int *)cfl_btree_search(tree, &k3);
    TEST_ASSERT(val != NULL);
    TEST_ASSERT_EQUAL_INT(20, *val);

    val = (int *)cfl_btree_search(tree, &k2);
    TEST_ASSERT(val != NULL);
    TEST_ASSERT_EQUAL_INT(5, *val);

    int k_missing = 99;
    TEST_ASSERT(cfl_btree_search(tree, &k_missing) == NULL);

    cfl_btree_free(tree, NULL);
}

TEST_CASE(test_cfl_btree_delete) {
    CFL_BTREEP tree = cfl_btree_new(3, compare_int_keys);
    int keys[] = {1, 2, 3};

    for(int i=0; i<3; i++) cfl_btree_add(tree, &keys[i]);

    // TEST_ASSERT_EQUAL_INT(3, cfl_btree_numNodes(tree));

    cfl_btree_delete(tree, &keys[1]); // Remove 2

    // TEST_ASSERT_EQUAL_INT(2, cfl_btree_numNodes(tree));
    TEST_ASSERT(cfl_btree_search(tree, &keys[1]) == NULL);
    TEST_ASSERT_EQUAL_INT(1, *(int*)cfl_btree_search(tree, &keys[0]));

    cfl_btree_free(tree, NULL);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_btree_lifecycle);
    RUN_TEST(test_cfl_btree_add_find);
    RUN_TEST(test_cfl_btree_delete);
TEST_SUITE_END()
