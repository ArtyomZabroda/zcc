#ifndef COMPILER_VM_H_
#define COMPILER_VM_H_

#include "chunk.h"
#include <stdio.h>

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    char* file_name;
    FILE* fptr;
} VM;

typedef enum {
  COMPILE_OK,
  COMPILE_ERROR,
} CompileResult;

void InitVM(VM* vm, const char* file_name);
void FreeVM(VM* vm);
CompileResult Compile(VM* vm, Chunk* chunk);

#endif