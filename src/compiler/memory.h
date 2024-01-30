#ifndef COMPILER_MEMORY_H_
#define COMPILER_MEMORY_H_

#include <stddef.h>

void* Reallocate(void* pointer, size_t oldSize, size_t newSize);

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)Reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    Reallocate(pointer, sizeof(type) * (oldCount), 0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)



#endif