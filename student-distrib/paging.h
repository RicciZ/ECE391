#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PDE_SIZE        1024
#define PTE_SIZE        1024
#define SIZE_4KB        4096
#define SIZE_4MB        0x400000
#define VIDEO_MEM_ADDR  0xB8000
#define VIDEO_MEM_END   0xB8FFF
#define KERNEL_MEM_ADDR 0x400000
#define KERNEL_MEM_END  0x7FFFFF
#define PHY_USER_START  0x800000
#define VIR_USER_PRO    0x8000000
#define VIR_USER_END    0x8400000



// Page Directory Entry for 4-KiB
typedef struct pde_4K {
    union {
        uint32_t val;

        struct {
            uint8_t P:          1;
            uint8_t R:          1;
            uint8_t U:          1;
            uint8_t W:          1;
            uint8_t D:          1;
            uint8_t A:          1;
            uint8_t reserved_0: 1;
            uint8_t S:          1;
            uint8_t G:          1;
            uint8_t Avail:      3;
            uint32_t Address:   20;
        } __attribute__ ((packed));
    };
} pde_4K_t;



// Page Directory Entry for 4-MiB
typedef struct pde_4M {
    union {
        uint32_t val;

        struct{
            uint8_t P:          1;
            uint8_t R:          1;
            uint8_t U:          1;
            uint8_t W:          1;
            uint8_t D:          1;
            uint8_t A:          1;
            uint8_t reserved_0: 1;
            uint8_t S:          1;
            uint8_t G:          1;
            uint8_t Avail:      3;
            uint32_t reserved:  10;
            uint32_t Address:   10;
        } __attribute__ ((packed));
    };
} pde_4M_t;



// Page Directory Entry with 4-Kib and 4-Mib combined
typedef struct pde {
    union {
        pde_4K_t pde_4K;
        pde_4M_t pde_4M;
    } __attribute__ ((packed));
} pde_t;



// Page Table Entry
typedef struct pte {
    union {
        uint32_t val;

        struct {
            uint8_t P:          1;
            uint8_t R:          1;
            uint8_t U:          1;
            uint8_t W:          1;
            uint8_t C:          1;
            uint8_t A:          1;
            uint8_t D:          1;
            uint8_t reserved_0: 1;
            uint8_t G:          1;
            uint8_t Avail:      3;
            uint32_t Address:   20;
        };
    } __attribute__ ((packed));
} pte_t;



// create instance for page directory, page tables and video memory
pde_t page_dir[PDE_SIZE] __attribute__((aligned (SIZE_4KB)));
pte_t page_tbl[PTE_SIZE] __attribute__((aligned (SIZE_4KB)));
pte_t page_tbl_user[PTE_SIZE] __attribute__((aligned (SIZE_4KB)));



// declarations of paging-related functions
void init_paging ();
int32_t process_paging (int32_t pid);
void terminal_backup (int32_t tid);
void terminal_video ();
void vidmem_paging (int32_t address);
void vidmem_disable();
void heap_setup();


#endif
