#include "cfl_test.h"
#include "cfl_map_str.h"

TEST_CASE(test_cfl_mapstr_lifecycle) {
    CFL_MAPSTRP map = cfl_mapstr_new();
    TEST_ASSERT(map != NULL);
    TEST_ASSERT_EQUAL_INT(0, cfl_mapstr_length(map));
    cfl_mapstr_free(map);
}

TEST_CASE(test_cfl_mapstr_set_get) {
    CFL_MAPSTRP map = cfl_mapstr_new();

    cfl_mapstr_set(map, "key1", "value1");
    cfl_mapstr_set(map, "key2", "value2");

    TEST_ASSERT_EQUAL_INT(2, cfl_mapstr_length(map));

    TEST_ASSERT_EQUAL_STRING("value1", cfl_mapstr_get(map, "key1"));
    TEST_ASSERT_EQUAL_STRING("value2", cfl_mapstr_get(map, "key2"));

    const char * missing = cfl_mapstr_get(map, "key3");
    TEST_ASSERT(missing == NULL);

    TEST_ASSERT_EQUAL_STRING("default", cfl_mapstr_getDefault(map, "key3", "default"));

    cfl_mapstr_free(map);
}

TEST_CASE(test_cfl_mapstr_del) {
    CFL_MAPSTRP map = cfl_mapstr_new();
    cfl_mapstr_set(map, "k", "v");

    TEST_ASSERT(cfl_mapstr_del(map, "k"));
    TEST_ASSERT_EQUAL_INT(0, cfl_mapstr_length(map));

    cfl_mapstr_free(map);
}

TEST_CASE(test_cfl_mapstr_format) {
    CFL_MAPSTRP map = cfl_mapstr_new();
    cfl_mapstr_setFormat(map, "k", "val %d", 123);

    TEST_ASSERT_EQUAL_STRING("val 123", cfl_mapstr_get(map, "k"));

    cfl_mapstr_free(map);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_mapstr_lifecycle);
    RUN_TEST(test_cfl_mapstr_set_get);
    RUN_TEST(test_cfl_mapstr_del);
    RUN_TEST(test_cfl_mapstr_format);
TEST_SUITE_END()
