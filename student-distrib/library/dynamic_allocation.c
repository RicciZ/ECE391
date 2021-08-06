#include "dynamic_allocation.h"
#include "lib.h"
#include "../paging.h"



uint8_t magic_num[4] = {0x7A,0x33,0x9,0x4D};    // for boundary check, chosen randomly by my heart...  



/*
 * void init_heap()
 * inputs:          none
 * return value:    none
 * outputs:         initialize the heap area
 * notes:           
 */
void init_heap(){
    // initialize the heap infomation structure
    heap_info.first = NULL;
    heap_info.last = NULL;
    heap_info.num_area = 0;
    heap_info.num_byte = 0;
    heap_info.size_allocator = 12;
    heap_info.size_magicnum = 4;

    // set up paging
    heap_setup();
}



/*
 * void malloc(int32_t size)
 * inputs:          the size of the desired area
 * return value:    a pointer to the allocated area, NULL on failure 
 * outputs:         allocates an area to the user
 * notes:           
 */
void* malloc(int32_t size){
    if (size <= 0) return NULL;

    // the real size needed
    int32_t size_real = heap_info.size_allocator + size + heap_info.size_magicnum;
    int32_t i;  // loop index

    // check if the heap is empty
    if (heap_info.last == NULL){
        // allocate from the head of the heap
        allocator_t* first_ptr = (allocator_t*)HEAP_START;
        first_ptr->next = NULL;
        first_ptr->prev = NULL;
        first_ptr->size = size;
        
        // update the heap info
        heap_info.first = first_ptr;
        heap_info.last = first_ptr;
        heap_info.num_area++;
        heap_info.num_byte += size_real;

        // fill in the magic number
        uint8_t* boundary_ptr = (uint8_t*)((int32_t)first_ptr + heap_info.size_allocator + size);
        for (i = 0; i < 4; ++i){
            *(boundary_ptr + i) = magic_num[i];
        }

        return (void*)((int32_t)(first_ptr) + heap_info.size_allocator);
    }

    
    // the heap is not empty, try to add the new area to the end of linked list
    int32_t end = (int32_t)heap_info.last + heap_info.size_allocator + heap_info.last->size + heap_info.size_magicnum - 1;
    if ((HEAP_END - end) >= size_real){ // we can add it to the end of linked list
        // new allocator
        allocator_t* new_last = (allocator_t*)(end + 1);
        new_last->next = NULL;
        new_last->prev = heap_info.last;
        new_last->size = size;
        new_last->prev->next = new_last;

        // update the heap info
        heap_info.last = new_last;
        heap_info.num_area++;
        heap_info.num_byte += size_real;

        // fill in the magic number
        uint8_t* boundary_ptr = (uint8_t*)((int32_t)new_last + heap_info.size_allocator + size);
        for (i = 0; i < 4; ++i){
            *(boundary_ptr + i) = magic_num[i];
        }

        return (void*)((int32_t)(new_last) + heap_info.size_allocator);   
    }

    else{   // search the memory from start of the heap
        if ((HEAP_END - HEAP_START + 1) - heap_info.num_byte < size_real){  // not enough memory
            return NULL;
        }

        // check the memory before the first allocated area
        if (((int32_t)heap_info.first - HEAP_START) >= size_real){  // enough memory!
            // allocate from the head of the heap
            allocator_t* first_ptr = (allocator_t*)HEAP_START;
            first_ptr->next = heap_info.first;
            first_ptr->prev = NULL;
            first_ptr->size = size;
            first_ptr->next->prev = first_ptr;
        
            // update the heap info
            heap_info.first = first_ptr;
            heap_info.num_area++;
            heap_info.num_byte += size_real;

            // fill in the magic number
            uint8_t* boundary_ptr = (uint8_t*)((int32_t)first_ptr + heap_info.size_allocator + size);
            for (i = 0; i < 4; ++i){
                *(boundary_ptr + i) = magic_num[i];
            }

            return (void*)((int32_t)(first_ptr) + heap_info.size_allocator);
        }

        // walk through the list to find adequate memory, if possible
        allocator_t* cur_allocator = heap_info.first;

        while (cur_allocator != heap_info.last){
            int32_t cur_end = (int32_t)cur_allocator + heap_info.size_allocator + cur_allocator->size
                                 + heap_info.size_magicnum - 1;
            if (((int32_t)(cur_allocator->next) - cur_end - 1) >= size_real){   // enough memory!
                // new allocator
                allocator_t* new_ptr = (allocator_t*)(cur_end + 1);
                new_ptr->next = cur_allocator->next;
                new_ptr->prev = cur_allocator;
                new_ptr->size = size;
                cur_allocator->next = new_ptr;
                new_ptr->next->prev = new_ptr;

                // update the heap info
                heap_info.num_area++;
                heap_info.num_byte += size_real;

                // fill in the magic number
                uint8_t* boundary_ptr = (uint8_t*)((int32_t)new_ptr + heap_info.size_allocator + size);
                for (i = 0; i < 4; ++i){
                    *(boundary_ptr + i) = magic_num[i];
                }

                return (void*)((int32_t)(new_ptr) + heap_info.size_allocator);
            }

            // the current chunk is not big enough
            cur_allocator = cur_allocator->next;
        }
    }

    // cannot find adequate memory chunk, fail gracefully
    return NULL;
};



/*
 * int32_t free (void* ptr)
 * inputs:          the pointer to the allocated area
 * return value:    0 on success and -1 on failure
 * outputs:         free the allocated region
 * notes:           
 */
int32_t free (void* ptr){
    if ((int32_t)(ptr) < (HEAP_START + heap_info.size_allocator) || 
                        (int32_t)(ptr) > (HEAP_END - heap_info.size_magicnum)){
        return -1;
    }

    allocator_t* cur_allocator = (allocator_t*)((int32_t)(ptr) - heap_info.size_allocator);
    allocator_t* next_allocator = cur_allocator->next; 
    allocator_t* prev_allocator = cur_allocator->prev;

    if ((next_allocator == NULL) && (prev_allocator == NULL)){  // this chunk is the only allocated area
        heap_info.first = NULL;
        heap_info.last = NULL;
        heap_info.num_area = 0;
        heap_info.num_byte = 0;
        return 0;
    }

    if (next_allocator == NULL){        // this chunk is at the end of the linked list
        prev_allocator->next = NULL;
        heap_info.last = prev_allocator;
    }
    else if (prev_allocator == NULL){   // this chunk is at the start of the linked list
        next_allocator->prev = NULL;
        heap_info.first = next_allocator;
    }
    else{                               // this chunk is in the middle of the linked list
        next_allocator->prev = prev_allocator;
        prev_allocator->next = next_allocator;
    }

    // update heap info
    heap_info.num_area--;
    heap_info.num_byte -= (heap_info.size_allocator + cur_allocator->size + heap_info.size_magicnum);

    return 0;
};



/*
 * int32_t validate (void* ptr)
 * inputs:          the pointer to the allocated area
 * return value:    0 on success and -1 on failure
 * outputs:         validate the allocated chunk
 * notes:           
 */
int32_t validate(void* ptr){
    if ((int32_t)(ptr) < (HEAP_START + heap_info.size_allocator) || 
                        (int32_t)(ptr) > (HEAP_END - heap_info.size_magicnum)){
        return -1;
    }

    allocator_t* cur_allocator = (allocator_t*)((int32_t)(ptr) - heap_info.size_allocator);
    uint8_t* boundary = (uint8_t*)((int32_t)(ptr) + cur_allocator->size);   // pointer to the magic number

    int32_t i;  // loop index
    for (i = 0; i < 4; ++i){
        if (magic_num[i] != *(boundary + i)){
            return -1;
        }
    }
    
    return 0;
}



/*
 * void print_heap_info()
 * inputs:          none
 * return value:    none
 * outputs:         fprint all the heap-related info
 * notes:           
 */
void print_heap_info(){
    printf("Number of allocated memory chunks:  %d\n", heap_info.num_area);
    printf("Number of allocated memory size:    %d\n", heap_info.num_byte);

    if (heap_info.first == NULL) return;    // check if the list is empty

    int32_t i = 0;  // loop index
    allocator_t* cur_allocator = heap_info.first;

    while (cur_allocator != NULL) { // walk through the list
        int32_t cur_address = (int32_t)(cur_allocator) + heap_info.size_allocator;
        printf("Chunk %d, %d bytes, starting address: %d.\n", i, cur_allocator->size, cur_address);
        cur_allocator = cur_allocator->next;
        ++i;
    }
}



/*
 * void realloc (void* ptr)
 * inputs:          pointer
 * return value:    none
 * outputs:         reallocate
 * notes:           
 */
void* realloc(void* ptr, int32_t new_size){
    if ((int32_t)(ptr) < (HEAP_START + heap_info.size_allocator) || 
                        (int32_t)(ptr) > (HEAP_END - heap_info.size_magicnum)){
        return NULL;
    }

    // get the current allocator
    allocator_t* cur_allocator = (allocator_t*)((int32_t)(ptr) - heap_info.size_allocator);

    // allocate new memory chunk
    void* new_ptr = malloc(new_size);
    if (new_ptr == NULL) return NULL;   // return NULL on failure

    int32_t i;  // loop index
    for(i = 0; i < cur_allocator->size && i < new_size; ++i){
        *((uint8_t*)(new_ptr + i)) = *((uint8_t*)(ptr + i));
    }

    free(ptr);  // free the old memory chunk

    return new_ptr;
}
