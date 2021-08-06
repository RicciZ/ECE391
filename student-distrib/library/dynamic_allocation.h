#include "../types.h"



/* I choose 36 MB -- 40 MB to be the heap region */
#define HEAP_START      0x2400000
#define HEAP_END        0x27FFFFF



/* the start of each allocated area is an allocator structure */
typedef struct allocator{
    struct allocator* next;     // the pointer to the next allocated area, if no such area, set to NULL
    struct allocator* prev;     // the pointer to the previous allocated area, if no such area, set to NULL
    int32_t size;               // the size of the allocated area
}allocator_t;

/* the infomation of the heap area */
typedef struct {
    allocator_t* first;
    allocator_t* last;
    int32_t num_area;
    int32_t num_byte;
    int32_t size_allocator;
    int32_t size_magicnum;
} heap_info_t;



/* global variable */
heap_info_t heap_info;



/* function declarations */
void init_heap();
void* malloc(int32_t size);
int32_t free (void* ptr);
int32_t validate(void* ptr);
void print_heap_info();
void* realloc(void* ptr, int32_t new_size);
