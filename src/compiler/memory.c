#include "memory.h"

#include <stdlib.h>

void* Reallocate(void* pointer, size_t old_size, size_t new_size) {
  if (new_size == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, new_size);

  // Выходим, если не удалось выделить память 
  if (result == NULL) exit(1);

  return result;
}