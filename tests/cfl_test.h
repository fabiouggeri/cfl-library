/**
 * @file cfl_test.h
 * @brief Simple header-only unit testing framework.
 *
 * This framework provides basic assertion macros and a test runner mechanism.
 * It is designed to be lightweight and portable, requiring no external
 * dependencies.
 */

#ifndef CFL_TEST_H_
#define CFL_TEST_H_

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ANSI Color Codes */
#define CFL_TEST_COLOR_RESET "\x1b[0m"
#define CFL_TEST_COLOR_RED "\x1b[31m"
#define CFL_TEST_COLOR_GREEN "\x1b[32m"
#define CFL_TEST_COLOR_YELLOW "\x1b[33m"
#define CFL_TEST_COLOR_BLUE "\x1b[34m"

typedef struct {
  int tests_run;
  int tests_failed;
  int tests_passed;
} cfl_test_stats_t;

static cfl_test_stats_t _cfl_test_stats = {0, 0, 0};

/* --- Macros for Assertions --- */

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf(CFL_TEST_COLOR_RED                                                \
             "  [FAIL] %s:%d: Assertion failed: %s" CFL_TEST_COLOR_RESET "\n", \
             __FILE__, __LINE__, #cond);                                       \
      _cfl_test_stats.tests_failed++;                                          \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_MSG(cond, msg)                                             \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf(CFL_TEST_COLOR_RED                                                \
             "  [FAIL] %s:%d: Assertion failed: %s (%s)" CFL_TEST_COLOR_RESET  \
             "\n",                                                             \
             __FILE__, __LINE__, #cond, msg);                                  \
      _cfl_test_stats.tests_failed++;                                          \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual)                                \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf(CFL_TEST_COLOR_RED                                                \
             "  [FAIL] %s:%d: Expected %d, but got %d" CFL_TEST_COLOR_RESET    \
             "\n",                                                             \
             __FILE__, __LINE__, (int)(expected), (int)(actual));              \
      _cfl_test_stats.tests_failed++;                                          \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual)                             \
  do {                                                                         \
    if (strcmp((expected), (actual)) != 0) {                                   \
      printf(                                                                  \
          CFL_TEST_COLOR_RED                                                   \
          "  [FAIL] %s:%d: Expected '%s', but got '%s'" CFL_TEST_COLOR_RESET   \
          "\n",                                                                \
          __FILE__, __LINE__, (expected), (actual));                           \
      _cfl_test_stats.tests_failed++;                                          \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define TEST_PASS()                                                            \
  do {                                                                         \
    _cfl_test_stats.tests_passed++;                                            \
    /* printf(CFL_TEST_COLOR_GREEN "  [PASS]" CFL_TEST_COLOR_RESET "\n"); */   \
  } while (0)

/* --- Macros for Test Definition and Execution --- */

#define TEST_CASE(name)                                                        \
  void name(void);                                                             \
  void name(void)

#define RUN_TEST(test_func)                                                    \
  do {                                                                         \
    int failed_before = _cfl_test_stats.tests_failed;                          \
    printf("Running %s...", #test_func);                                       \
    test_func();                                                               \
    if (_cfl_test_stats.tests_failed == failed_before) {                       \
      printf(CFL_TEST_COLOR_GREEN " OK" CFL_TEST_COLOR_RESET "\n");            \
      TEST_PASS();                                                             \
    }                                                                          \
    _cfl_test_stats.tests_run++;                                               \
  } while (0)

#define TEST_SUITE_BEGIN()                                                     \
  int main(void) {                                                             \
    setvbuf(stdout, NULL, _IONBF, 0);                                          \
    printf(CFL_TEST_COLOR_BLUE "Starting Test Suite..." CFL_TEST_COLOR_RESET   \
                               "\n");                                          \
    clock_t start_time = clock();

#define TEST_SUITE_END()                                                       \
  clock_t end_time = clock();                                                  \
  double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;      \
  printf(CFL_TEST_COLOR_BLUE "Test Suite Completed." CFL_TEST_COLOR_RESET      \
                             "\n");                                            \
  printf("Tests Run: %d\n", _cfl_test_stats.tests_run);                        \
  printf("Tests Passed: " CFL_TEST_COLOR_GREEN "%d" CFL_TEST_COLOR_RESET "\n", \
         _cfl_test_stats.tests_passed);                                        \
  printf("Tests Failed: " CFL_TEST_COLOR_RED "%d" CFL_TEST_COLOR_RESET "\n",   \
         _cfl_test_stats.tests_failed);                                        \
  printf("Time Taken: %.4f seconds\n", time_taken);                            \
  return (_cfl_test_stats.tests_failed > 0 ? 1 : 0);                           \
  }

#endif /* CFL_TEST_H_ */
