#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "interrupt/keyboard.h"
#include "filesys.h"
#include "paging.h"
#include "library/lib.h"
#include "interrupt/sys_call.h"
#include "terminal.h"



#define MAX_FILES           8
#define MAX_PROCESS         6
#define MAGIC_NUM_SIZE      4
#define ADDR_SIZE           4
#define LOADING_ADDR        0x8048000
#define KERNEL_STACK_SIZE   0x2000
#define USER_STACK          (0x8400000 - 4)
#define SCREEN_START        0x9000000



// defintions of some common structures

typedef struct {
    int32_t (*open)(const uint8_t*);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
    int32_t (*close)(int32_t);
} file_operations_391_t;



typedef struct fd {
    file_operations_391_t* operation_pointer;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fd_t;



typedef struct PCB{
    fd_t fd_array[MAX_FILES];
    int32_t pid;           // if pid = -1, then this PCB is unused
    int32_t parent_pid;
    uint32_t kesp;          // kernel stack pointer
    uint32_t kebp;          // kernel base pointer
    uint32_t esp;           // user stack pointer
    uint32_t ebp;           // user base pointer
    uint8_t arg[BUFFER_SIZE];
    int32_t flag_vidmem;
    volatile int32_t flag_exception;
    terminal_t* terminal_ptr;
} PCB_t;



// declarations of helper functions (for system_execute)

void init_PCB ();
PCB_t* get_PCB(int32_t pid);
int32_t process_create (const uint8_t* command);
int32_t process_terminate(uint8_t status);
int32_t init_fd(fd_t* fd_array_in);
int32_t schedule();
#endif
