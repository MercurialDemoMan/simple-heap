#include <stdint.h>

typedef uint64_t u64;

/**
 * \brief place heap at the beggining of the data segment
 */
void  heap_init();
/**
 * allocate new space in heap
 */
void* heap_alloc(u64 size);
/**
 * allocate new space in heap and zero out the data
 */
void* heap_calloc(u64 size);
/**
 * reallocate space in heap
 */
void* heap_realloc(void* data, u64 size);
/**
 * \brief remove space in heap
 */
void  heap_free(void* data);
/**
 * \brief print heap state
 */
void  heap_print();
/**
 * \brief reset heap
 */
void  heap_reset();
