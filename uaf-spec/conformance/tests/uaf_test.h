#ifndef UAF_TEST_H
#define UAF_TEST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int uaf_fail_count;
static const char *uaf_case_name = "?";
#define CASE(name) (uaf_case_name = (name))
#define CHECK(cond) do { \
    if (!(cond)) { uaf_fail_count++; \
        printf("  FAIL [%s] %s:%d: %s\n", uaf_case_name, __FILE__, __LINE__, #cond); } \
  } while (0)
#define CHECK_EQ_I(a, b) do { long long _a = (long long)(a), _b = (long long)(b); \
    if (_a != _b) { uaf_fail_count++; \
        printf("  FAIL [%s] %s:%d: %s == %s (%lld != %lld)\n", \
               uaf_case_name, __FILE__, __LINE__, #a, #b, _a, _b); } \
  } while (0)
#define CHECK_EQ_U(a, b) do { unsigned long long _a = (unsigned long long)(a), \
                              _b = (unsigned long long)(b); \
    if (_a != _b) { uaf_fail_count++; \
        printf("  FAIL [%s] %s:%d: %s == %s (0x%llX != 0x%llX)\n", \
               uaf_case_name, __FILE__, __LINE__, #a, #b, _a, _b); } \
  } while (0)
#define TEST_MAIN_END(suite) do { \
    if (uaf_fail_count == 0) { printf("PASS %s\n", (suite)); return 0; } \
    printf("FAIL %s (%d checks failed)\n", (suite), uaf_fail_count); return 1; \
  } while (0)
#endif
