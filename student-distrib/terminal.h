#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"

#define BUFFER_SIZE         128
#define TERMINAL_NUM        3

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_init();
void terminal_color_init();
void terminal_switch(int32_t new_ter);
void search_history(int32_t flag);
void clear_history();



// structure for terminal information
typedef struct terminal
{
    /* 
    *   a flag indicating the current running environment
    *   0 --- keyboard buffering
    *   1 --- terminal
    */
    uint8_t flag_function;
    uint8_t keyboard_buf[BUFFER_SIZE];
    uint8_t read_count;
    int32_t tid;
    volatile int32_t input_done;
    int screen_x;
    int screen_y;
    int32_t pid;
    uint32_t video_mem_buf;     //video memory buffer
    uint8_t (*history)[BUFFER_SIZE];
    int32_t history_num;
    int32_t history_index;
    int32_t history_size;

}terminal_t;



terminal_t terminal_array[TERMINAL_NUM];
terminal_t* running_terminal;   //running terminal
terminal_t* display_terminal;   //display terminal


#endif
