#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"
#include "process.h"

#define MAX_FILENAME_LEN    32
#define DENTRY_RESERVE      24
#define BOOTBLK_RESERVE     52
#define NUM_FILES           63
#define BLOCK_SIZE          (4*1024)        // 4kB
#define RTC_FILE            0
#define DIR_FILE            1
#define REG_FILE            2

// directory entry i.e. dentry 64B
typedef struct dentry
{
    char        name[MAX_FILENAME_LEN];     // 32B
    uint32_t    type;                       // 4B
    uint32_t    inode;                      // 4B
    uint8_t     reserved[DENTRY_RESERVE];   // 24B
} dentry_t;                                 // 64B

// boot block 4kB
typedef struct bootblock
{
    uint32_t    num_dentries;               // 4B
    uint32_t    num_inodes;                 // 4B
    uint32_t    num_data_blocks;            // 4B
    uint8_t     reserved[BOOTBLK_RESERVE];  // 52B
    dentry_t    dentries[NUM_FILES];        // 64B * 63
} boot_blk_t;                              // 4kB

// inode 4kB
typedef struct inode
{
    uint32_t    size;                       // 4B
    uint32_t    data[BLOCK_SIZE/4-1];       // 4B * 1023
} inode_t;                                  // 4kB

// data block 4kB
typedef struct data_blk
{
    uint8_t     data[BLOCK_SIZE];           // 4kB
} data_blk_t;

/* initialize the file system */
void fs_init(void* fs);

/* Open the regular file. */
int32_t file_open (const uint8_t* filename);

/* Close the regular file. */
int32_t file_close (int32_t fd);

/* Read the contents in a file. */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);

/* Read only. No use.*/
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);

/* Open the directory file. */
int32_t dir_open (const uint8_t* filename);

/* Close the directory file. */
int32_t dir_close (int32_t fd);

/* Read one file's file name in the directory file into the buffer. */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

/* Read only. No use. */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);

/* read info of a file to the dentry by name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

/* read info of a file to the dentry by index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

/* read data of a file to the buf based on offset and length */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
