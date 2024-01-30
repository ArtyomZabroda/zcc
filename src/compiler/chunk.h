#ifndef COMPILER_CHUNK_H_
#define COMPILER_CHUNK_H_

#include "value.h"
#include <stddef.h>
#include <stdint.h>

// Однобайтные коды операций (operation code) соответствующие инструкциям
typedef enum {
  OP_CONST_INT,
} OpCode;

// Динаминческий массив байтов
typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  ValueArray constants;
} Chunk;

// "Конструктор" для динамического массива байтов
void InitChunk(Chunk* chunk);

// "Деструктор" для динамического массива байтов
void FreeChunk(Chunk* chunk);

// Добавить указанный байт в конец
void PushBackChunk(Chunk* chunk, uint8_t byte, int line);

// Переместить данные в новый участок памяти
void* Reallocate(void* pointer, size_t old_size, size_t new_size);

// Добавить константу в дин. массив констант данного чанка. Возвращает позицию константы в массиве.
int AddConstant(Chunk* chunk, Value value);
#endif // COMPILER_CHUNK_H_