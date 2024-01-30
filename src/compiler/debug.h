#ifndef COMPILER_DEBUG_H_
#define COMPILER_DEBUG_H_

#include "chunk.h"

void DisassembleChunk(Chunk* chunk, const char* name);
int DisassembleInstruction(Chunk *chunk, int offset);

#endif