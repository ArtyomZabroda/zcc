#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "chunk.h"


void InitVM(VM* vm, const char* file_name) {
  vm->file_name = strdup(file_name);
  vm->fptr = fopen(file_name, "w");
}

void FreeVM(VM* vm) {
  free(vm->file_name);
  fclose(vm->fptr);
}

static CompileResult run(VM* vm) {
}

int NumPlaces (int n) {
    int r = 1;
    if (n < 0) n = (n == INT_MIN) ? INT_MAX: -n;
    while (n > 9) {
        n /= 10;
        r++;
    }
    return r;
}

CompileResult Compile(VM* vm, Chunk* chunk) {
  vm->chunk = chunk;
  vm->ip = vm->chunk->code;

  int code_size = 41;
  char* code = malloc(code_size);
  strcat(code, "section .data\n");
  int constants_count = vm->chunk->constants.count;
  for (size_t i = 0; i < constants_count; ++i) {
    Value v = vm->chunk->constants.values[i];
    int symbols_number_in_line = 12 + NumPlaces(i) + NumPlaces(v);
    // TODO: Исправить, жутко неэффективно
    code_size += symbols_number_in_line;
    code = realloc(code, symbols_number_in_line + code_size);
    char formatted_str[symbols_number_in_line];
    snprintf(formatted_str, symbols_number_in_line, "   c%zu: dd %i\n", i, v);
    strcat(code, formatted_str);
  }
  strcat(code, "section .bss\n");
  strcat(code, "section .text\n");

  CompileResult r = run(vm);

  fprintf(vm->fptr, "%s", code);
  free(code);
  return r;
}




