#include "cfl_test.h"
#include "cfl_hash.h"
#include "cfl_str.h"

// Hash function for strings
static CFL_UINT32 my_hash_str(void *k) {
    char *str = (char *)k;
    CFL_UINT32 hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// Equality function for strings
static int my_eq_str(void *k1, void *k2) {
    return strcmp((char *)k1, (char *)k2) == 0;
}

TEST_CASE(test_cfl_hash_lifecycle) {
    printf("  [DEBUG] test_cfl_hash_lifecycle start\n");
    CFL_HASHP hash = cfl_hash_new(10, my_hash_str, my_eq_str, NULL);
    printf("  [DEBUG] cfl_hash_new returned %p\n", hash);
    TEST_ASSERT(hash != NULL);
    printf("  [DEBUG] Checking count\n");
    TEST_ASSERT_EQUAL_INT(0, cfl_hash_count(hash));
    printf("  [DEBUG] Freeing hash\n");
    cfl_hash_free(hash, CFL_FALSE);
    printf("  [DEBUG] test_cfl_hash_lifecycle end\n");
}

TEST_CASE(test_cfl_hash_insert_search_remove) {
    printf("  [DEBUG] test_cfl_hash_insert_search_remove start\n");
    CFL_HASHP hash = cfl_hash_new(10, my_hash_str, my_eq_str, NULL);

    char key1[] = "key1";
    char val1[] = "value1";
    char key2[] = "key2";
    char val2[] = "value2";

    // Insert
    printf("  [DEBUG] Insert key1\n");
    TEST_ASSERT(cfl_hash_insert(hash, key1, val1));
    printf("  [DEBUG] Insert key2\n");
    TEST_ASSERT(cfl_hash_insert(hash, key2, val2));
    printf("  [DEBUG] Checking count\n");
    TEST_ASSERT_EQUAL_INT(2, cfl_hash_count(hash));

    // Search
    printf("  [DEBUG] Search key1\n");
    char *found1 = (char *)cfl_hash_search(hash, "key1");
    printf("  [DEBUG] Search key2\n");
    char *found2 = (char *)cfl_hash_search(hash, "key2");
    printf("  [DEBUG] Search key3\n");
    char *not_found = (char *)cfl_hash_search(hash, "key3");

    TEST_ASSERT_EQUAL_STRING("value1", found1);
    TEST_ASSERT_EQUAL_STRING("value2", found2);
    TEST_ASSERT(not_found == NULL);

    // Remove
    printf("  [DEBUG] Remove key1\n");
    char *removed = (char *)cfl_hash_remove(hash, "key1");
    TEST_ASSERT_EQUAL_STRING("value1", removed);
    TEST_ASSERT_EQUAL_INT(1, cfl_hash_count(hash));

    printf("  [DEBUG] Verify removal\n");
    TEST_ASSERT(cfl_hash_search(hash, "key1") == NULL);

    printf("  [DEBUG] Freeing hash\n");
    cfl_hash_free(hash, CFL_FALSE);
    printf("  [DEBUG] test_cfl_hash_insert_search_remove end\n");
}

TEST_CASE(test_cfl_hash_iterator) {
    CFL_HASHP hash = cfl_hash_new(10, my_hash_str, my_eq_str, NULL);
    cfl_hash_insert(hash, "A", "1");
    cfl_hash_insert(hash, "B", "2");
    cfl_hash_insert(hash, "C", "3");

    CFL_ITERATORP it = cfl_hash_iterator(hash);
    TEST_ASSERT(it != NULL);

    int count = 0;
    while(cfl_iterator_hasNext(it)) {
        CFL_HASH_ENTRYP entry = (CFL_HASH_ENTRYP)cfl_iterator_next(it);
        TEST_ASSERT(entry != NULL);
        count++;
    }
    TEST_ASSERT_EQUAL_INT(3, count);

    cfl_iterator_free(it);
    cfl_hash_free(hash, CFL_FALSE);
}

TEST_SUITE_BEGIN()
    RUN_TEST(test_cfl_hash_lifecycle);
    printf("lifecycle passed\n");
    RUN_TEST(test_cfl_hash_insert_search_remove);
    printf("insert_search_remove passed\n");
    RUN_TEST(test_cfl_hash_iterator);
    printf("iterator passed\n");
TEST_SUITE_END()
