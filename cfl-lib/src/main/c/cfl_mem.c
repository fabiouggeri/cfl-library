#include <string.h>

#include "cfl_mem.h"

typedef struct {
   CFL_MALLOC_FUNC  malloc_func;
   CFL_REALLOC_FUNC realloc_func;
   CFL_FREE_FUNC    free_func;
} CFL_MEM_FUNCTIONS, *CFL_MEM_FUNCTIONSP;

static CFL_MEM_FUNCTIONS mem_functions = { malloc,
                                           realloc,
                                           free };

void cfl_mem_set(CFL_MALLOC_FUNC malloc_func, CFL_REALLOC_FUNC realloc_func, CFL_FREE_FUNC free_func) {
   mem_functions.malloc_func = malloc_func;
   mem_functions.realloc_func = realloc_func;
   mem_functions.free_func = free_func;
}

void * cfl_malloc(size_t size) {
   return mem_functions.malloc_func(size);
}

void * cfl_calloc(size_t numElements, size_t size) {
   void *ptr = mem_functions.malloc_func(numElements * size);
   if (ptr != NULL) {
      memset(ptr, 0, numElements * size);
   }
   return ptr;
}

void * cfl_realloc(void *ptr, size_t size) {
   return mem_functions.realloc_func(ptr, size);
}

void cfl_free(void *ptr) {
   mem_functions.free_func(ptr);
}
