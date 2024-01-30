#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

static int SimpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int ConstIntInstruction(const char* name, Chunk* chunk,
                               int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  PrintValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2; // Инструкция занимает 2 байта 
}

int DisassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  // Показывать  номер линии или | в случае, если номер линии повторяется
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset-1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONST_INT:
      return ConstIntInstruction("OP_CONST_INT", chunk, offset);
    default:
      printf("Неизвестный код операции %d\n", instruction);
      return offset + 1;
  }
}

void DisassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);
  for (int offset = 0; offset < chunk->count;) {
    offset = DisassembleInstruction(chunk, offset);
  }
}