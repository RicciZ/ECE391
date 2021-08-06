#include "process.h"
#include "x86_desc.h"


int32_t process_counter = 0;    // counts the number of existing process 
int32_t running_process = -1;   // records the current running process, -1 indicates no running process
fd_t* fd_array;                 // the file descriptor array of current porcess
extern tss_t tss;               
int32_t shells_booted = 0;          // a flag indicating whether all three basic shells have been booted



/* 
 *  PCB_t* get_PCB(int32_t pid)
 *  DESCRIPTION: get a pointer to the PCB corresponding to the pid
 *  INPUTS:     pid
 *  OUTPUTS:    none
 *  RETURN VALUE: return a pointer, or null on failure
 */
PCB_t* get_PCB(int32_t pid){
    if(pid < 0 || pid >= MAX_PROCESS) return NULL;
    // the first kernel stack is located at the bottom of the kernel page
    return (PCB_t*)((KERNEL_MEM_END + 1) - (pid + 1) * KERNEL_STACK_SIZE);
}



/* 
 *  int32_t get_new_pid ()
 *  DESCRIPTION: allocate a pid for the new process
 *  INPUTS:     none
 *  OUTPUTS:    none
 *  RETURN VALUE: return a pid, or -1 on failure
 */
int32_t get_new_pid(){
    if (process_counter >= MAX_PROCESS) return -1;
    int32_t i;  // loop index
    for (i = 0; i < MAX_PROCESS; ++i){
        PCB_t* ptr = get_PCB(i);
        if (ptr->pid == -1){    // -1 indicates this block is unused
            ptr->pid = i;
            return i;
        }
    }
    return -1;  // no valid pid found
}



/* 
 *  void init_PCB ()
 *  DESCRIPTION: initialize all PCBs
 *  INPUTS:     none
 *  OUTPUTS:    set all the PCB to unused, i.e., pid = -1
 *  RETURN VALUE: none
 */
void init_PCB(){
    uint32_t i;  // loop index
    for (i = 0; i < MAX_PROCESS; ++i){
        PCB_t* ptr = get_PCB(i);
        ptr->pid = -1;
    }
}



/* 
 *  void init_fd (fd_t* fd_array_in)
 *  DESCRIPTION: initialize the file descriptor
 *  INPUTS:     the pointer to the fd
 *  OUTPUTS:    
 *  RETURN VALUE: 0 on success, -1 on failure
 */
int32_t init_fd(fd_t* fd_array_in){
    //check whether input is NULL 
    if (fd_array_in == NULL){
        return -1;
    }
    extern file_operations_391_t terminal_operation;
    fd_array = fd_array_in;

    //initialize, 0 for stdin
    fd_array[0].operation_pointer = &terminal_operation;
    fd_array[0].flags = 1;
    fd_array[0].inode = 0;
    fd_array[0].file_position = 0;

    //1 for stdout
    fd_array[1].operation_pointer = &terminal_operation;
    fd_array[1].flags = 1;
    fd_array[1].inode = 0;
    fd_array[1].file_position = 0;

    int i;  // loop index
    for(i = 2; i < MAX_FILES; i ++){
        fd_array[i].flags = 0;
    }
    return 0;
}



/* 
 *  void switch_fd (fd_t* fd_array_in)
 *  DESCRIPTION: update the global variable fd_array
 *  INPUTS:     the pointer to the fd
 *  OUTPUTS:    
 *  RETURN VALUE: 0 on success, -1 on failure
 */
int32_t switch_fd(fd_t* fd_array_in){
    if (fd_array_in == NULL){
        return -1;
    }
    fd_array = fd_array_in;
    return 0;
}



/* 
 *  int32_t process_create (const uint8_t* command)
 *  DESCRIPTION: create a new process based on the command
 *  INPUTS:     command arguments
 *  OUTPUTS:    parse the arguments, set up paging, load the program and initialize the corresponding PCB 
 *  RETURN VALUE: return 0 on success, -1 on failure 
 */
int32_t process_create (const uint8_t* command){
    cli();

    if (!command) return -1;

    int32_t rval;

    // parse args
    uint8_t file_name[MAX_FILENAME_LEN + 1];
    uint8_t arg[BUFFER_SIZE];

    int file_name_idx = 0;
    int arg_idx = 0;
    int cmd_idx = 0;

    while (command[cmd_idx] == ' ') cmd_idx++;
    if (command[cmd_idx] == '\0') return -1;        // no file name
    while (command[cmd_idx] != ' ' && command[cmd_idx] != '\0'){
        if (file_name_idx > MAX_FILENAME_LEN) return -1;
        file_name[file_name_idx++] = command[cmd_idx++];
    }
    file_name[file_name_idx] = '\0';
    while (command[cmd_idx] == ' ') cmd_idx++;
    while (command[cmd_idx] != ' ' && command[cmd_idx] != '\0'){
        arg[arg_idx++] = command[cmd_idx++];
    }
    arg[arg_idx] = '\0';

    // executable check
    int i;
    dentry_t dentry;
    uint8_t magic_num_file[MAGIC_NUM_SIZE];
    uint8_t magic_num[MAGIC_NUM_SIZE] = {0x7f,0x45,0x4c,0x46};
    for (i = 0; i < MAGIC_NUM_SIZE; i++) magic_num_file[i] = '\0';
    if (read_dentry_by_name(file_name,&dentry) == -1) return -1;
    if (read_data(dentry.inode,0,magic_num_file,4) == -1) return -1;
    for (i = 0; i < MAGIC_NUM_SIZE; i++){
        if (magic_num_file[i] != magic_num[i]) return -1;
    }

    uint8_t prog_ptr[ADDR_SIZE];                    // the entry point of the program
    read_data(dentry.inode,24,prog_ptr,4);          // 24-27 are virtual address of the instruction to be executed
    int32_t entry_point = 0;
    for (i = 0; i < ADDR_SIZE; ++i){
        entry_point |= prog_ptr[i] << (i * 8);
    }

    if (process_counter >= MAX_PROCESS) {
        char message[28] = "Maximum Processes Reached.\n";
        int32_t temp = 0;
        while (message[temp] != '\0'){
            putc_visible(message[temp]);
            temp++;
        }
        return 1;
    }

    // allocate pid and PCB
    int32_t pid = get_new_pid();
    if (pid == -1) return -1;
    PCB_t* PCB_ptr = get_PCB(pid);

    // fill in PCB info
    if (init_fd(get_PCB(pid)->fd_array) == -1) return -1;
    if (process_counter < 3){
        PCB_ptr->parent_pid = -1;
    }
    else{
        PCB_ptr->parent_pid = running_process;
    }
    PCB_ptr->esp = USER_STACK;
    PCB_ptr->ebp = USER_STACK;
    PCB_ptr->kesp = (KERNEL_MEM_END + 1) - pid * KERNEL_STACK_SIZE - 4;
    PCB_ptr->kebp = (KERNEL_MEM_END + 1) - pid * KERNEL_STACK_SIZE - 4;
    strcpy((int8_t*)PCB_ptr->arg, (int8_t*)arg);
    PCB_ptr->flag_vidmem = 0;
    PCB_ptr->flag_exception = 0;
    PCB_ptr->terminal_ptr = running_terminal;
    PCB_ptr->terminal_ptr->pid = pid;

    // set up the paging mapping for new process
    process_paging(pid);

    // user-level process loader
    uint8_t* load_buf = (uint8_t*)LOADING_ADDR;
    extern inode_t* inode_start;
    if (read_data(dentry.inode, 0, load_buf, inode_start[dentry.inode].size) == -1) return -1;

    // update TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PCB_ptr->kesp;
    
    // check if this is the first process
    if (running_process != -1){
        PCB_t* parent_ptr = get_PCB(running_process);
        asm volatile("movl %%esp, %0":"=r" (parent_ptr->kesp));
        asm volatile("movl %%ebp, %0":"=r" (parent_ptr->kebp));
    }

    // update running process flag and process counter
    running_process = pid;
    process_counter++;

    sti();

    // IRET context switch, reference: OSDEV, set up EIP, CS, EFLAGS, ESP and DS
    // magic number: DS: 0x2B, CS: 0x23, USER_STACK: (0x8400000 - 4) = 0x83FFFFC
    asm volatile (
        "mov $0x2B, %%ax;"
        "mov %%ax, %%ds;"
        "pushl $0x2B;"
        "movl $0x83FFFFC, %%eax;"
        "pushl %%eax;"
        "pushfl;"
        "popl %%eax;"
        "orl $0x200, %%eax;"
        "pushl %%eax;"
        "pushl $0x23;"
        "pushl %1;"
        "iret;"
        "sys_exe_return:;"
        "movl %%eax, %0;"

        : "=g" (rval)
        : "r" (entry_point)
        : "eax", "edx"
    );
    return rval;
}



/* 
 *  int32_t process_terminate (uint8_t status)
 *  DESCRIPTION: terminate a process and do some clean up
 *  INPUTS:     status
 *  OUTPUTS:    
 *  RETURN VALUE: should not return to its caller, return -1 on failure
 */
int32_t process_terminate(uint8_t status){
    cli();
    // get the current PCB
    PCB_t* PCB_ptr = get_PCB(running_process);
    if (PCB_ptr == NULL) {
        printf("No process is running.\n");
        return -1;
    }

    // get the parent pid
    int32_t parent = PCB_ptr->parent_pid;
    if (parent == -1){  // check if this is the first process
        printf("\n*********Shell Rebooted*******\n\n");
        running_process = -1;
        process_counter--;
        PCB_ptr->pid = -1;
        process_create((uint8_t*)"shell");
        return -1;
    }
    else{               // this process is not the first process
        // get the parent PCB
        PCB_t* parent_ptr = get_PCB(parent);

        // update TSS
        tss.ss0 = KERNEL_DS;
        tss.esp0 = (KERNEL_MEM_END + 1) - parent * KERNEL_STACK_SIZE - 4;

        // close all related files 
        int32_t i;
        for (i = 0; i < MAX_FILES; i++) close(i);
        switch_fd(parent_ptr->fd_array);

        // set up process paging
        if (PCB_ptr->flag_vidmem == 1) {vidmem_disable();}
        process_paging(parent);

        // update the terminal-related info
        running_terminal->pid = parent;

        // mark this PCB unused and update the running process
        PCB_ptr->pid = -1;
        running_process = parent;
        process_counter--;

        // check if this halt is called from exception
        if (PCB_ptr->flag_exception == 1){
            // jump to system_execute return
            asm volatile(
                "xorl %%eax, %%eax;"
                "movl $256, %%eax;"
                "movl %0, %%ebp;"
                "movl %1, %%esp;"
                "sti;"
                "jmp sys_exe_return;"
                :
                : "r" (parent_ptr->kebp), "r" (parent_ptr->kesp)
                : "eax", "ebp", "esp"
            );
        }
        else{
            // jump to system_execute return
            asm volatile(
                "xorl %%eax, %%eax;"
                "movb %0, %%al;"
                "movl %1, %%ebp;"
                "movl %2, %%esp;"
                "sti;"
                "jmp sys_exe_return;"
                :
                : "r" (status), "r" (parent_ptr->kebp), "r" (parent_ptr->kesp)
                : "eax", "ebp", "esp"
            );
        }
    }

    return -1;
}



/*
*   int32_t schedule()
*   input:          none
*   return value:   0 on success, -1 on failure
*   output:         make switch between processes
*/
int32_t schedule (){
    cli();
    // check whether all three basic shells have been booted
    if (shells_booted == 0){    // not all shells have been booted
        current_color++;
        running_terminal = &terminal_array[running_process + 1];

        if (process_counter == 0){
            terminal_video();
        }
        else{
            terminal_backup(process_counter);
        }
        if (process_counter == 2) { // all basic shells will have been set up by the end of this function
            shells_booted = 1;
        }

        // create the basic shell
        process_create((uint8_t*)"shell");
        sti();
        return 0;
    }

    // all three basic shells have been booted
    int32_t cur_tid = running_terminal->tid;
    int32_t new_tid = cur_tid + 1;
    if (new_tid == 3)   new_tid = 0;    // only 3 terminals allowed

    // save the current kernel context
    PCB_t* cur_PCB = get_PCB(running_process);
    asm volatile("movl %%esp, %0":"=r" (cur_PCB->kesp));
    asm volatile("movl %%ebp, %0":"=r" (cur_PCB->kebp));

    // set up context for switching process
    int32_t next_pid = terminal_array[new_tid].pid;
    running_process = next_pid;
    running_terminal = &terminal_array[new_tid];
    PCB_t* next_PCB = get_PCB(next_pid);
    switch_fd(next_PCB->fd_array);
    extern int32_t terminal_switched;
    terminal_switched = 0;
    current_color = new_tid;

    // set up the paging mapping for new process
    process_paging(next_pid);
    if (running_terminal == display_terminal){
        terminal_video();
        if (next_PCB->flag_vidmem == 1){
            vidmem_paging(VIDEO_MEM_ADDR);
        }
    }
    else{
        terminal_backup(new_tid);
        if (next_PCB->flag_vidmem == 1){
            vidmem_paging(running_terminal->video_mem_buf);
        }
    }

    // update TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (KERNEL_MEM_END + 1) - next_pid * KERNEL_STACK_SIZE - 4;

    // check if current process should be killed
    extern int32_t halt_terminal;
    if (halt_terminal == running_terminal->tid){
        halt_terminal = -1;
        process_terminate(0);
    }

    // set up the kernel esp and ebp
    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        : "r" (next_PCB->kesp), "r" (next_PCB->kebp)
        : "memory"
    );

    sti();
    return 0;
}
