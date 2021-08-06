#include "filesys.h"
#include "library/lib.h"

boot_blk_t* boot_blk_ptr;
dentry_t* den_start;
inode_t* inode_start;
data_blk_t* data_blk_start;

extern fd_t* fd_array;



/*
*   void fs_init()
*   Inputs:         the pointer to the starting address of the file
*   Return value:   none
*   Outputs:        initialize some related pointers
*/
void fs_init(void* fs){
    boot_blk_ptr = fs;
    den_start = &((dentry_t*)boot_blk_ptr)[1];
    inode_start = &((inode_t*)boot_blk_ptr)[1];
    data_blk_start = &((data_blk_t*)boot_blk_ptr)[boot_blk_ptr->num_inodes + 1];
}



/*
*   int32_t read_dentry_by_name
*   Inputs:         fname -- the name of the file needed to be read
*                   dentry -- the sturct stored the file information
*   Return value:   return 0 on success, or return -1 on failure
*   Outputs:        none
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    //check input validity
    if (!fname) return -1;
    if (!dentry) return -1;
    if (strlen((int8_t*)fname) > MAX_FILENAME_LEN || strlen((int8_t*)fname) <= 0) return -1;

    int i;
    uint32_t len = (strlen((int8_t*)fname) == MAX_FILENAME_LEN) ? MAX_FILENAME_LEN : strlen((int8_t*)fname) + 1;
    dentry_t* den_ptr = den_start;
    //find the file which has matched name, copy the file information
    for (i = 0; i < NUM_FILES; i++){
        if (strncmp((int8_t*)fname,(den_ptr+i)->name,len)) continue;
        *dentry = *(den_ptr + i);
        return 0;
    }
    return -1;
}



/*
*   int32_t read_dentry_by_index
*   Inputs:         index -- the index in boot block
*                   dentry -- the sturct stored the file information
*   Return value:   return 0 on success, or return -1 on failure
*   Outputs:        none
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    //check index validity
    if((index >= boot_blk_ptr->num_dentries) || (index < 0)) return -1;
    *dentry = *(den_start + index);
    return 0;
}



/*
*   int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
*   Inputs:         inode is associated with the desired file, offset is the starting 
*                   position, buf is destination buffer and length is the required 
*                   number of bytes
*   Return value:   return the number of bytes read, or return -1 on failure
*   Outputs:        read data from the file into the buffer
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // validate inode and buffer
    if (inode < 0 || inode > (boot_blk_ptr->num_inodes - 1)) return -1;
    if (buf == NULL) return -1;

    inode_t* inode_ptr = (inode_t*)(inode_start + inode);
    // calculate the number of used data blocks
    uint32_t num_block = (inode_ptr->size / BLOCK_SIZE);
    if ((inode_ptr->size & 0xFFF) != 0) ++num_block;        // mod

    uint32_t i;     // loop index
    // validate each used data block
    for (i = 0; i < num_block; ++i){
        if (inode_ptr->data[i] < 0 || inode_ptr->data[i] > (boot_blk_ptr->num_data_blocks - 1)) return -1;
    }

    // read the file 
    if (offset >= inode_ptr->size) return 0;
    uint32_t blk_index = offset / BLOCK_SIZE;   // the index of the first block to read
    // the pointer to the current data block 
    data_blk_t* data_blk_cur = (data_blk_t*)(data_blk_start + inode_ptr->data[blk_index]);   
    int32_t num_read = 0;                      // the number of bytes being read
    while ((offset + num_read) < inode_ptr->size && num_read < length){
        if ((offset + num_read)/ BLOCK_SIZE > blk_index){   // we should go to the next data block
            ++blk_index;
            data_blk_cur = (data_blk_t*)(data_blk_start + inode_ptr->data[blk_index]);
        }
        // read the data to the buffer, 0xFFF is used as a modulo operation
        buf[num_read] = data_blk_cur->data[(offset + num_read) & 0xFFF];   
        ++num_read; 
    }
    return num_read;
}



/* file_open
 * 
 * Open the regular file.
 * Inputs:  file name
 * Outputs: 0 on success and -1 on failure.
 * Side Effects: None
 */
int32_t file_open (const uint8_t* filename) {
    return 0;
}



/* file_close
 * 
 * Close the regular file.
 * Inputs: file descriptor number.
 * Outputs: 0 for success and -1 for failure.
 * Side Effects: None
 */
int32_t file_close (int32_t fd) {
    return 0;
}



/* file_read
 * 
 * Read the contents in a file.
 * Inputs: file descriptor number, the destination buffer and number of bytes to read
 * Outputs: return the number of bytes read. 
 * Side Effects: None
 */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes) {
    int32_t len = read_data(fd_array[fd].inode, fd_array[fd].file_position, buf, nbytes);
    if (len == -1) return -1;
    fd_array[fd].file_position += len;
    return len;
}



/* file_write
 * 
 * Read only.
 * Inputs: file descriptor number, the source buffer and the number of bytes to write
 * Outputs: return -1 by default (read-only system)
 * Side Effects: None
 */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/* dir_open
 * 
 * Open the directory file.
 * Inputs:  file name
 * Outputs: 0 on success and -1 on failure.
 * Side Effects: None
 */
int32_t dir_open (const uint8_t* filename) {
    return 0;
}



/* dir_close
 * 
 * Close the directory file.
 * Inputs: file descriptor number.
 * Outputs: 0 for success and -1 for failure.
 * Side Effects: None
 */
int32_t dir_close (int32_t fd) {
    return 0;
}



/* dir_read
 * 
 * Read one file's file name in the directory file into the buffer.
 * Inputs: file descriptor number, the destination buffer and number of bytes to read
 * Outputs: return the number of bytes read. 
 * Side Effects: None
 */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes) {
    if (!buf) return -1;

    dentry_t den;
    if (fd_array[fd].file_position >= boot_blk_ptr->num_dentries) return 0;
    if (read_dentry_by_index(fd_array[fd].file_position, &den)) return -1;
    strncpy(buf, den.name, MAX_FILENAME_LEN);
    fd_array[fd].file_position++;
    ((char*)buf)[MAX_FILENAME_LEN] = '\0';
    int32_t len = (int32_t)strlen(buf);
    if (len < 0 || len > MAX_FILENAME_LEN) return -1;
    ((char*)buf)[len] = '\0';
    return len;
}


/* dir_write
 * 
 * Read only.
 * Inputs: file descriptor number, the source buffer and the number of bytes to write
 * Outputs: return -1 by default (read-only system)
 * Side Effects: None
 */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
