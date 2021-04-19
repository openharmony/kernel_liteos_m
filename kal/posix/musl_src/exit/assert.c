#include "assert.h"
#include "los_context.h"
#include "los_debug.h"


void __assert(const char* file, int line, const char* expr) {
  PRINT_ERR("__assert error: %s, %d, %s\n", file, line, expr);
  LOS_Panic("__assert error:\n");
  while (1);
}

void __assert2(const char* file, int line, const char* func, const char* expr) {
  PRINT_ERR("%s:%d: %s: assertion \"%s\" failed\n", file, line, func, expr);
  LOS_Panic("__assert error:\n");
  while (1);
}

void __assert_fail(const char* expr, const char* file, int line, const char* func) {
  PRINT_ERR("%s:%d: %s: assertion \"%s\" failed\n", file, line, func, expr);
  LOS_Panic("__assert error:\n");
  while (1);
}
