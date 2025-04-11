#ifndef DYN_ARRAY_H_
#define DYN_ARRAY_H_

#include <string.h>
#include <assert.h>

#define DYNARRAY_INIT(array, nr, alloc) \
  nr = 0; \
  alloc = 0; \
  array = NULL \

#define ALLOC_NR(capacity) \
  ((capacity) < 8 ? 8 : (capacity) * 2)
  
#define DYNARRAY_REALLOC(array, alloc_nr) array = realloc(array, sizeof(*array) * alloc_nr)

#define DYNARRAY_GROW(x, nr, alloc) \
	do { \
		if ((nr) > alloc) { \
			if (ALLOC_NR(alloc) < (nr)) \
				alloc = (nr); \
			else \
				alloc = ALLOC_NR(alloc); \
        DYNARRAY_REALLOC(x, alloc); \
		} \
	} while (0)

#define DYNARRAY_ADD(array, nr, alloc, elem) \
  ++nr; \
  DYNARRAY_GROW(array, nr, alloc); \
  array[nr - 1] = elem

#define DYNARRAY_FREE(array, nr, alloc) \
  free(array); \
  nr = 0; \
  alloc = 0 \

#endif
