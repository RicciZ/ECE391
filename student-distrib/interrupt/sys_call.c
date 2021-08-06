#include "sys_call.h"

extern int32_t process_counter;     // counts the number of existing process 
extern int32_t running_process;     // records the current running process, -1 indicates no running process
extern fd_t* fd_array;              // the file descriptor array of current porcess



/*  operation tables */

file_operations_391_t rtc_operation = {
    .open = rtc_open,
    .read = rtc_read,
    .write = rtc_write,
    .close = rtc_close
};

file_operations_391_t dir_operation = {
    .open = dir_open,
    .read = dir_read,
    .write = dir_write,
    .close = dir_close
};

file_operations_391_t file_operation = {
    .open = file_open,
    .read = file_read,
    .write = file_write,
    .close = file_close
};

file_operations_391_t terminal_operation = {
    .open = terminal_open,
    .read = terminal_read,
    .write = terminal_write,
    .close = terminal_close
};



/*  system call functions   */

/* 
 *  int32_t halt (uint8_t status)
 *  DESCRIPTION: system call -- halt
 *  INPUTS:     status
 *  OUTPUTS:    halt a process, and do some clean up
 *  RETURN VALUE: should not return to its caller
 */
int32_t halt (uint8_t status){
    process_terminate(status);
    return -1;
};



/* 
 *  int32_t execute (const uint8_t* command)
 *  DESCRIPTION: system call -- execute
 *  INPUTS:     command
 *  OUTPUTS:    create a new process
 *  RETURN VALUE: 0 on success, -1 on failure
 */
int32_t execute (const uint8_t* command){
    return process_create(command);
};



/* 
 *  int32_t read (int32_t fd, void* buf, int32_t nbytes)
 *  DESCRIPTION: system call -- read
 *  INPUTS:     fd -- file descriptor index
 *              buf -- buffer used to store read context
 *              nbytes -- the number of bytes need to be read
 *  OUTPUTS:    none
 *  RETURN VALUE: the number of bytes have been read, -1 for failure
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    //check input validity,
    if(fd < 0 || fd >= MAX_FILES || (!buf) || nbytes < 0 || 1 == fd){
        return -1;
    }

    //the file is closed
    if(0 == fd_array[fd].flags){
        return -1;
    }

    return fd_array[fd].operation_pointer->read(fd,buf,nbytes);
}



/* 
 *  int32_t write (int32_t fd, void* buf, int32_t nbytes)
 *  DESCRIPTION: system call -- write
 *  INPUTS:     fd -- file descriptor index
 *              buf -- buffer stored context need to be write
 *              nbytes -- the number of bytes need to be write
 *  OUTPUTS:    none
 *  RETURN VALUE: the number of bytes have been write, -1 for failure
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    //check input validity
    if(fd < 1 || fd >= MAX_FILES || (!buf) || nbytes < 0){
        return -1;
    }
    //the file is closed
    if(0 == fd_array[fd].flags){
        return -1;
    }

    return fd_array[fd].operation_pointer->write(fd,buf,nbytes);
}



/* 
 *  int32_t open (const uint8_t* filename)
 *  DESCRIPTION: open the file, store into file descriptor
 *  INPUTS:     filename -- the name of file
 *  OUTPUTS:    none
 *  RETURN VALUE: index of file descriptor for success, -1 for failure
 */
int32_t open (const uint8_t* filename){

    //check input validity
    if(filename == NULL){
        return -1;
    }

    int32_t i;
    //check whether available file descriptor space
    for (i = 2;i < MAX_FILES; i++){
        if (0 == fd_array[i].flags){
            break;
        }  
    }
    if (MAX_FILES == i){
        return -1;
    }

    //check whether the neamed file exist
    dentry_t temp_dentry;
    uint32_t file_type;

    if(-1 == read_dentry_by_name(filename,&temp_dentry)){
        return -1;
    }

    //initialize the file descriptor
    file_type = temp_dentry.type;
    fd_array[i].flags = 1;
    fd_array[i].inode = 0;
    fd_array[i].file_position = 0;

    switch (file_type)
    {
    case RTC_FILE:
        fd_array[i].operation_pointer = &rtc_operation;
        break;
    case DIR_FILE:
        fd_array[i].operation_pointer = &dir_operation;
        break;
    case REG_FILE:
        fd_array[i].inode = temp_dentry.inode;
        fd_array[i].operation_pointer = &file_operation;
        break;
    default:
        fd_array[i].flags = 0;
        return -1;
    }

    //check whether open success
    if(-1 == fd_array[i].operation_pointer->open(filename)){
        return -1;
    }

    return i;
}



/* 
 *  int32_t close (int32_t fd)
 *  DESCRIPTION: close the file, clear corresponding file descriptor
 *  INPUTS:     fd -- the index of file descriptor
 *  OUTPUTS:    none
 *  RETURN VALUE: 0 for success, -1 for failure
 */
int32_t close (int32_t fd){
    //check input vadility, 0,1 for default descriptor which can't be closed
    if(fd < 2 || fd >= MAX_FILES){
        return -1;
    }
    //the file is closed before
    if(0 == fd_array[fd].flags){
        return -1;
    }
    fd_array[fd].flags = 0;
    return fd_array[fd].operation_pointer->close(fd);
}



/* 
 *  int32_t getargs (uint8_t* buf, int32_t nbytes)
 *  DESCRIPTION: store nbytes of args from pcb->arg to buf
 *  INPUTS:     buf -- to store the arg
 *              nbytes -- length of arg we want
 *  OUTPUTS:    none
 *  RETURN VALUE: 0 for success, -1 for failure
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    if (!buf) return -1;
    PCB_t* pcb = get_PCB(running_process);
    if (!pcb) return -1;
    if (pcb->arg[0] == '\0') return -1;
    int arg_idx = 0;
    while (arg_idx + 1 < nbytes && pcb->arg[arg_idx] != '\0'){
        buf[arg_idx] = pcb->arg[arg_idx];
        arg_idx++;
    }
    buf[arg_idx] = '\0';
    return 0;
}



/* 
 *  int32_t vidmap (uint8_t** screen_start)
 *  DESCRIPTION:    maps the text-mode video memory into user space at 
 *                  a pre-set virtual address.
 *  INPUTS:         double pointer to the start of the user video-memory address
 *  OUTPUTS:        may change the physical video memory
 *  RETURN VALUE:   video memory address for user programs,
 *                  0 on success, -1 on failure.
 */
int32_t vidmap (uint8_t** screen_start){
    // validate the argument
    if ((uint32_t)screen_start < VIR_USER_PRO || (uint32_t)screen_start >= VIR_USER_END) return -1;
    if (running_process == -1) return -1;

    PCB_t* ptr = get_PCB(running_process);
    vidmem_paging(VIDEO_MEM_ADDR);        // set up the mapping, note the start of the screen is at 144MB (virtual)
    *screen_start = (uint8_t*)SCREEN_START;
    ptr->flag_vidmem = 1;
    return 0;
};



/*** extra credit ***/
int32_t set_handler (int32_t signum, void* handler_address){return -1;};
int32_t sigreturn (void){return -1;};
