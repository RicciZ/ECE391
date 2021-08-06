#include "terminal.h"
#include "types.h"
#include "interrupt/keyboard.h"
#include "library/lib.h"
#include "library/cursor.h"
#include "paging.h"
#include "library/dynamic_allocation.h"


int32_t terminal_switched = 0;


/*
 * void terminal_open (const uint8_t* filename)
 * inputs:          const uint8_t* filename
 * return value:    0 on success
 * outputs:         initialize the terminal specific variables
 * notes:           
 */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}



/*
 * void terminal_close (int32_t fd)
 * inputs:          int32_t fd
 * return value:    0 on success
 * outputs:         clear the terminal specific variables
 * notes:           
 */
int32_t terminal_close(int32_t fd){
    return -1;
}



/*
 * void terminal_write (int32_t fd, const void* buf, int32_t nbytes)
 * inputs:          buf is the source buffer and nbytes is the required number of bytes to be written
 * return value:    the number of bytes written to the screen
 * outputs:         put the contents of buffer onto screen
 * notes:           
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    running_terminal->flag_function = 1;            // tell the keyboard handler the current environment is terminal
    if (buf == NULL) return -1; // check the pointer validity
    int32_t i;      // loop index
    for (i = 0; i < nbytes; ++i){
        if (*(uint8_t*)(buf + i) == '\0') continue;     // ignore '\0'
        putc(*(uint8_t*)(buf + i));
    }
    running_terminal->flag_function = 0;            // tell the keyboard handler the writing is over 
    return i;
}



/*
 * void terminal_read (int32_t fd, const void* buf, int32_t nbytes)
 * inputs:          buf is the destination buffer and nbytes is the required number of bytes to be read
 * return value:    the number of bytes read from the keyboard buffer, including '\0'
 * outputs:         copy the contents of the keyboard buffer into the destination buf
 * notes:           
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    cli();
    if (buf == NULL) return -1; // check the pointer validity
    int32_t i;      // loop index
    if (running_terminal->read_count != 0){
        for (i = 0; i < running_terminal->read_count; ++i){
            putc(running_terminal->keyboard_buf[i]);
        }
    }
    running_terminal->flag_function = 1;    // tell the keyboard handler the current environment is terminal
    running_terminal->input_done = 0;       // tell the keyboard handler the input has not finished
    sti();
    // wait until the input is all typed in
    while (display_terminal != running_terminal || running_terminal->input_done == 0);  
    cli();  
    for (i = 0; i < nbytes; ++i){   // copy the contents of keyboard_buffer into the buf
        *(uint8_t*)(buf + i) = display_terminal->keyboard_buf[i]; 
        *(uint8_t*)(display_terminal->history[display_terminal->history_num] + i) = display_terminal->keyboard_buf[i]; 
        if (display_terminal->keyboard_buf[i] == '\n'){
            ++i;
            break;
        } 
    }
    clear_keyboard_buffer();    // clear the keyboard_buffer
    display_terminal->flag_function = 0;            // tell the keyboard handler the reading is over 

    // update the command history
    display_terminal->history_num++;
    display_terminal->history_index = display_terminal->history_num - 1;  
    if (display_terminal->history_num == display_terminal->history_size){
        void* temp_ptr;
        temp_ptr = realloc((void*)display_terminal->history, display_terminal->history_size * BUFFER_SIZE * 2);
        if (temp_ptr == NULL){
            printf("Command history overflow, please clear the buffer.\n");
        }
        else{
            display_terminal->history = (uint8_t (*)[BUFFER_SIZE])temp_ptr;
            display_terminal->history_size = 2 * display_terminal->history_size;
        }
    }
    sti();     
    return i;
}



/*
 * int32_t terminal_init()
 * inputs:          none
 * return value:    0 for success
 * outputs:         initialize the terminal, 3 terminals in total
 * notes:           
 */
int32_t terminal_init(){
    int i;
    //initialize terminal 
    for(i = 0; i < TERMINAL_NUM; i++){
        terminal_array[i].read_count = 0;
        terminal_array[i].flag_function = 0;
        terminal_array[i].screen_x = 0;
        terminal_array[i].screen_y = 0;
        terminal_array[i].video_mem_buf = VIDEO_MEM_ADDR + (i + 1) * SIZE_4KB;
        terminal_array[i].input_done = 0;
        terminal_array[i].tid = i;
        terminal_array[i].pid = -1;
        terminal_array[i].history = (uint8_t (*)[BUFFER_SIZE])malloc(BUFFER_SIZE * 10);
        terminal_array[i].history_num = 0;
        terminal_array[i].history_index = -1;
        terminal_array[i].history_size = 10;
    }

    //initialize running_terminal pointer and display_terminal pointer to first terminal
    running_terminal = &terminal_array[0];
    display_terminal = &terminal_array[0];

    return 0;
}



/*
 * int32_t terminal_color_init()
 * inputs:          none
 * return value:    none
 * outputs:         initialize the color of each terminal
 * notes:           
 */
void terminal_color_init(){
    //initialize the video memory for each terminal
    current_color = 0;
    terminal_video();
    clear();
    for(current_color = 1; current_color < 3; ++current_color){ // 3 terminals in total
        terminal_backup(current_color);
        int32_t i;
        extern char* video_mem;
        for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
            *(uint8_t *)(video_mem + (i << 1)) = ' ';
            *(uint8_t *)(video_mem + (i << 1) + 1) = color_scheme[current_color];
        }
    }
    current_color = -1;
    terminal_video();
}



/*
 * void terminal_switch (int32_t new_ter)
 * inputs:          int32_t the id of new terminal
 * return value:    none
 * outputs:         switch the terminal
 * notes:           
 */
void terminal_switch(int32_t new_ter){
    cli();
    // if new terminal is current termianl, return
    if (display_terminal == &terminal_array[new_ter] || terminal_switched == 1){
        sti();
        return;
    }

    if (running_terminal == display_terminal){  // virtual video memory maps to the physical video memory
        // store display terminal information
        memcpy((void*)(display_terminal->video_mem_buf), (const void*)VIDEO_MEM_ADDR, SIZE_4KB);

        // copy the content of backup memory to the physical videomemory
        memcpy((void*)VIDEO_MEM_ADDR, (const void*)terminal_array[new_ter].video_mem_buf, SIZE_4KB);
        terminal_backup(display_terminal->tid);
    }
    else{                                       // virtual video memory maps to one of the backup memories
        // copy the content of backup memory to the physical videomemory
        terminal_video();
        // store display terminal information
        memcpy((void*)(display_terminal->video_mem_buf), (const void*)VIDEO_MEM_ADDR, SIZE_4KB);
        memcpy((void*)VIDEO_MEM_ADDR, (const void*)terminal_array[new_ter].video_mem_buf, SIZE_4KB);
        terminal_backup(running_terminal->tid);
    }

    // update current terminal
    display_terminal = &terminal_array[new_ter];
    terminal_switched = 1;
    switch_cursor(display_terminal->screen_x, display_terminal->screen_y);
    sti();
}



/*
 * void search_history(int32_t flag)
 * inputs:          int32_t 
 * return value:    none
 * outputs:         search the command history
 * notes:           
 */
void search_history(int32_t flag){
    int32_t i;      // loop index

    if (display_terminal->history_index == -1 && flag == 1) return;

    // delete all the characters shown on the screen
    if (display_terminal->read_count != 0){
        for (i = 0; i < display_terminal->read_count; ++i){
            deletec(0);
        }
    }

    // get history
    if (flag == 1){ // get previous command
        i = 0;
        while (display_terminal->history[display_terminal->history_index][i] != '\n'){
            putc_visible(display_terminal->history[display_terminal->history_index][i]);
            display_terminal->keyboard_buf[i] = display_terminal->history[display_terminal->history_index][i];
            i++;
        }
        display_terminal->read_count = i;
        if (display_terminal->history_index != 0){
            display_terminal->history_index--;
        }
    }

    else{           // get next command
        if (display_terminal->history_index != display_terminal->history_num - 1){
            display_terminal->history_index++;
            i = 0;
            while (display_terminal->history[display_terminal->history_index][i] != '\n'){
                putc_visible(display_terminal->history[display_terminal->history_index][i]);
                display_terminal->keyboard_buf[i] = display_terminal->history[display_terminal->history_index][i];
                i++;
            }
            display_terminal->read_count = i;
        }
        else{
            display_terminal->read_count = 0;
        }
    }
}



/*
 * void clear_history()
 * inputs:          none
 * return value:    none
 * outputs:         clear the command history
 * notes:           
 */
void clear_history(){
    display_terminal->history_index = -1;
    display_terminal->history_num = 0;
    display_terminal->history_size = 10;
    free(display_terminal->history);
    display_terminal->history = (uint8_t (*)[BUFFER_SIZE])malloc(BUFFER_SIZE * 10);
}
