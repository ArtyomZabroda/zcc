#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void Crash(int exit_code, const char *const format, ...) {
  va_list argptr;
  va_start(argptr, format);
  fprintf(stderr, "error: ");
  vfprintf(stderr, format, argptr);
  fprintf(stderr, "\n");
  va_end(argptr);
  exit(exit_code);
}
