#include "cfl_test.h"
#include "cfl_map.h"
#include <string.h>

// Helper functions for map
static int compare_ints(const void *k1, const void *k2) {
    int i1 = *(const int*)k1;
    int i2 = *(const int*)k2;
    return i1 - i2;
}

TEST_CASE(test_cfl_map_lifecycle) {
    CFL_MAPP map = cfl_map_new(sizeof(int), sizeof(int), compare_ints, NULL);
    TEST_ASSERT(map != NULL);
    TEST_ASSERT_EQUAL_INT(0, cfl_map_length(map));
    cfl_map_free(map);
}

TEST_CASE(test_cfl_map_set_get) {
    CFL_MAPP map = cfl_map_new(sizeof(int), sizeof(int), compare_ints, NULL);

    int k1 = 1, v1 = 10;
    int k2 = 2, v2 = 20;

    cfl_map_set(map, &k1, &v1);
    cfl_map_set(map, &k2, &v2);

    TEST_ASSERT_EQUAL_INT(2, cfl_map_length(map));

    int *get1 = (int *)cfl_map_get(map, &k1);
    int *get2 = (int *)cfl_map_get(map, &k2);

    TEST_ASSERT(get1 != NULL);
    TEST_ASSERT_EQUAL_INT(10, *get1);
    TEST_ASSERT(get2 != NULL);
    TEST_ASSERT_EQUAL_INT(20, *get2);

    // Update
    int v3 = 30;
    cfl_map_set(map, &k1, &v3);
    get1 = (int *)cfl_map_get(map, &k1);
    TEST_ASSERT_EQUAL_INT(30, *get1);

    cfl_map_free(map);
}

TEST_CASE(test_cfl_map_del) {
    CFL_MAPP map = cfl_map_new(sizeof(int), sizeof(int), compare_ints, NULL);
    int k1 = 1, v1 = 10;
    cfl_map_set(map, &k1, &v1);

    TEST_ASSERT_EQUAL_INT(1, cfl_map_length(map));

    TEST_ASSERT(cfl_map_del(map, &k1));
    TEST_ASSERT_EQUAL_INT(0, cfl_map_length(map));
    TEST_ASSERT(cfl_map_get(map, &k1) == NULL);

    cfl_map_free(map);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_map_lifecycle);
    RUN_TEST(test_cfl_map_set_get);
    RUN_TEST(test_cfl_map_del);
TEST_SUITE_END()
