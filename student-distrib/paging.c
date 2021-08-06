#include "paging.h"
#include "types.h"
#include "library/lib.h"
#include "process.h"
#include "terminal.h"
#include "library/dynamic_allocation.h"



/*
 * void init_paging ()
 * inputs:          None
 * return value:    None  
 * outputs:         initialize the paging setup
 * notes:           page directory and page table are defined in paging.h
 */
void init_paging(){
		int32_t i;      // loop index

        // initialize the page directory entry to not present
        for (i = 0; i < PDE_SIZE; ++i)
            page_dir[i].pde_4M.val = 0;
        // initialize the page table entry to not present
        for (i = 0; i < PTE_SIZE; ++i)
            page_tbl[i].val = 0;

        // initialize the first 4 MB with the page table
        page_dir[0].pde_4K.P = 1;
        page_dir[0].pde_4K.R = 1;
        page_dir[0].pde_4K.U = 0;
        page_dir[0].pde_4K.W = 0;
        page_dir[0].pde_4K.D = 0;
        page_dir[0].pde_4K.A = 0;
        page_dir[0].pde_4K.reserved_0 = 0;
        page_dir[0].pde_4K.S = 0;
        page_dir[0].pde_4K.Address = (uint32_t) page_tbl >> 12; // only the highest 20 bits are needed

        // initialize the page table, particularly, initialize the video memory pte, and the other 3 backup memory
        for (i = 0; i < 4; ++i){
            int32_t address = VIDEO_MEM_ADDR + i * SIZE_4KB;
            page_tbl[address >> 12].P = 1;
            page_tbl[address >> 12].R = 1;
            page_tbl[address >> 12].U = 0;
            page_tbl[address >> 12].Address = address >> 12;    // only the highest 20 bits are needed
        }
        

        // initialize the second 4MB, which is the kernel memory
        page_dir[1].pde_4M.P = 1;
        page_dir[1].pde_4M.R = 1;
        page_dir[1].pde_4M.U = 0;
        page_dir[1].pde_4M.W = 0;
        page_dir[1].pde_4M.D = 0;
        page_dir[1].pde_4M.A = 0;
        page_dir[1].pde_4M.reserved_0 = 0;
        page_dir[1].pde_4M.S = 1;
        page_dir[1].pde_4M.Address = KERNEL_MEM_ADDR >> 22; // only the highest 10 bits are needed

        // enable the paging, refer to OSDev
        asm volatile(
			"movl %0, %%eax;"
			"movl %%eax, %%cr3;"
            "movl %%cr4, %%eax;"
            "orl $0x10, %%eax;"
            "movl %%eax, %%cr4;"
            "movl %%cr0, %%eax;"
            "orl $0x80000000, %%eax;"
            "movl %%eax, %%cr0;"
			:
			: "r" (page_dir)
			: "eax"
		);
}



/*
 * int32_t process_paging (int32_t pid)
 * inputs:          pid, indicating which process is being executed
 * return value:    return 0 on success, or -1 on failure  
 * outputs:         modify the paging mapping (virtual address 128 MB to the correct physical address)
 * notes:           page directory and page table are defined in paging.h
 */
int32_t process_paging (int32_t pid){
    // validate the pid number
    if (pid < 0 || pid >= MAX_PROCESS) return -1;

    // remap the user program paging
    uint32_t shift_offset = 22;     // since we are using 4 MB page, shift 22 bits
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.P = 1;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.R = 1;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.U = 1;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.W = 0;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.D = 0;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.A = 0;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.reserved_0 = 0;
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.S = 1;
    // the first user program is loaded at physical 8 MB, and the second at 12 MB, etc.
    page_dir[VIR_USER_PRO >> shift_offset].pde_4M.Address = (PHY_USER_START >> shift_offset) + pid;

    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );

    return 0;
}



/*
 * void vidmem_paging ()
 * inputs:          
 * return value:    none 
 * outputs:         maps the mem_start to the physical video memory
 * notes:           user page table is defined in paging.h
 */
void vidmem_paging (int32_t address){
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.P = 1;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.R = 1;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.U = 1;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.W = 0;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.D = 0;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.A = 0;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.reserved_0 = 0;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.S = 0;
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.Address = (uint32_t) page_tbl_user >> 12;  // only the highest 20 bits are needed

    page_tbl_user[0].P = 1;
    page_tbl_user[0].R = 1;
    page_tbl_user[0].U = 1;
    page_tbl_user[0].Address = address >> 12;    // only the highest 20 bits are needed

    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
}



/*
 * void vidmem_disable ()
 * inputs:          none
 * return value:    none 
 * outputs:         disable the vidmem paging
 * notes:           user page table is defined in paging.h
 */
void vidmem_disable(){
    page_dir[SCREEN_START / SIZE_4MB].pde_4K.P = 0;
    page_tbl_user[0].P = 0;

    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
}



/*
 * void terminal_backup ()
 * inputs:          int32_t tid
 * return value:    none 
 * outputs:         remaps the virtual video memory to one of the backup memories
 * notes:           page table is defined in paging.h
 */
void terminal_backup (int32_t tid){
    // virtual video memory maps to the backup physical memory
    int32_t address = VIDEO_MEM_ADDR + (tid + 1) * SIZE_4KB;
    page_tbl[VIDEO_MEM_ADDR >> 12].P = 1;
    page_tbl[VIDEO_MEM_ADDR >> 12].R = 1;
    page_tbl[VIDEO_MEM_ADDR >> 12].U = 0;
    page_tbl[VIDEO_MEM_ADDR >> 12].Address = address >> 12; // only the highest 20 bits are needed
    
    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
}



/*
 * void terminal_video ()
 * inputs:          none
 * return value:    none 
 * outputs:         remaps the virtual video memory to the physical video memory
 * notes:           page table is defined in paging.h
 */
void terminal_video (){
    page_tbl[VIDEO_MEM_ADDR >> 12].P = 1;
    page_tbl[VIDEO_MEM_ADDR >> 12].R = 1;
    page_tbl[VIDEO_MEM_ADDR >> 12].U = 0;
    page_tbl[VIDEO_MEM_ADDR >> 12].Address = VIDEO_MEM_ADDR >> 12;  // only the highest 20 bits are needed

    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
}



/*
 * void heap_setup() ()
 * inputs:          none
 * return value:    none 
 * outputs:         set up heap paging
 */
void heap_setup(){
    int32_t index = HEAP_START >> 22;
    page_dir[index].pde_4M.P = 1;
    page_dir[index].pde_4M.R = 1;
    page_dir[index].pde_4M.U = 1;
    page_dir[index].pde_4M.S = 1;
    page_dir[index].pde_4M.Address = HEAP_START >> 22; // only the highest 10 bits are needed

    // flush the TLB
    // reference: OSDEV
    asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
}
