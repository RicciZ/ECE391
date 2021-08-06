#include "sb16.h"

#define BLK_SIZE     (32*1024)
#define BUF_SIZE     (2*BLK_SIZE)

//Allocate a buffer that does not cross a 64k physical page boundary
static int8_t DMA_Buffer[BUF_SIZE] __attribute__((aligned(32768))) = {};
static int8_t *block1 = DMA_Buffer;
static int8_t *block2 = &(DMA_Buffer[BLK_SIZE]);
static int8_t *cur_block = DMA_Buffer;
static int8_t audio_filename[33];
static uint32_t current_offset;
static int8_t is_playing = 0;
static uint32_t audio_file_inode = 0;

// I/O port addresses for lower page registers, channel 4 is unused
static int8_t page_ports[8] = {0x87, 0x83, 0x81, 0x82, 0x00, 0x8B, 0x89, 0x8A};

int32_t _fd;

void DSP_outb(uint8_t data, uint8_t port_offset){
    outb(data, SB16_IOBase + port_offset);
}

uint8_t DSP_inb(uint8_t port_offset){
    return inb(SB16_IOBase + port_offset);
}

/*
 * int8_t Reset_DSP()
 * 
 * Description: Write a 1 to the reset port (2x6)
 *              Wait for 3 microseconds
 *              Write a 0 to the reset port (2x6)
 *              Poll the read-buffer status port (2xE) until bit 7 is set
 *              Poll the read data port (2xA) until you receive an AA
 * Input:       None
 * Output:      0 for success
 * SideEffect:  None
 * 
 */
int8_t Reset_DSP(){
    uint32_t _3ms = 1000 * 0.03;
    uint8_t i = 0;                  // counter
    uint8_t read_val = 0;
    DSP_outb(1, DSP_Reset);
    while(i < _3ms){
        i ++;
    }
    DSP_outb(0, DSP_Reset);
    while(!(read_val & (1 << 7))) read_val = DSP_inb(DSP_Read_Buffer_Status);
    while(!(read_val & 0xAA)) read_val = DSP_inb(DSP_Read);

    return 0;
}

/*
 * int8_t Write_DSP(uint8_t data)
 * 
 * Description: Read the write-buffer status port (2xC) until bit 7 is cleared
 *              Write the value to the write port (2xC)
 * Input:       Data written to the port
 * Output:      0 for success
 * SideEffect:  None
 * 
 */

int8_t Write_DSP(uint8_t data){
    volatile uint8_t read_val = 0;
    while(1){
        read_val = DSP_inb(DSP_Write);
        if(!(read_val & (1 << 7))){
            break;
        }
    }

    DSP_outb(data, DSP_Write);

    return 0;
}

/*
 * int8_t Read_From_DSP()
 * 
 * Description: Read the read-buffer status port (2xE) until bit 7 is set
 *              Read the value from the read port (2xA)
 * Input:       None
 * Output:      Data read out.
 * SideEffect:  None
 * 
 */
uint8_t Read_From_DSP(){
    uint8_t read_val = 0;
    while(!(read_val & (1 << 7))) read_val = DSP_inb(DSP_Read_Buffer_Status);

    return DSP_inb(DSP_Read);
}

/*
 * int8_t Program_DMA_Controller(uint8_t channel, uint8_t mode, uint32_t addr, uint32_t size)
 * 
 * Description: Read the read-buffer status port (2xE) until bit 7 is set
 *              Read the value from the read port (2xA)
 * Input:       None
 * Output:      0 for success
 * SideEffect:  None
 * 
 */

int8_t Program_DMA_Controller(uint8_t channel, uint8_t mode, uint32_t addr, uint32_t size){
    uint32_t count = size;

    // 1. Disable the sound card DMA channel by setting the appropriate mask bit 
    outb(channel | DMA_BIT_2, DMA_1_MASK);

    // 2. Clear the byte pointer flip-flop 
    outb(0, DMA_1_CLEAR_PTR);

    // 3. Write the DMA mode for the transfer 
    outb(mode | channel, DMA_1_MODE);

    // 4. Write the offset of the buffer, 
    //    low byte followed by high byte. 
    //    For sixteen bit data, the offset 
    //    should be in words from the start of a 128kbyte page. 
    //    The easiest method for computing 16-bit parameters is to 
    //    divide the linear address by two before calculating offset

    //   Set paging first
    outb(addr >> 16, page_ports[channel]);

    //   After that , executing step 4
    outb(addr & 0xFF, DMA_1_BASE + (channel << 1));
    outb((addr >> 8) & 0xFF, DMA_1_BASE + (channel << 1));

    // 5. Write the transfer length, low byte followed by high 
    //    byte. For an 8-bit transfer, write the number of bytes-1. 
    //    For a 16-bit transfer, write the number of words-1.
    count--;
    outb(count & 0xFF, DMA_1_BASE + (channel << 1) + 1);
    outb((count >> 8) & 0xFF, DMA_1_BASE + (channel << 1) + 1);

    // 6.Enable the sound card DMA channel by clearing the appropriate mask bit
    outb(channel, DMA_1_MASK);

    return 0;
}


/*
 * int8_t Set_Sample_Rate(uint16_t frequency)
 * 
 * Description: Write the high byte of the sampling rate (56h for 22050 hz)
 *              Write the low byte of the sampling rate (22h for 22050 hz)
 * Input:       frequency
 * Output:      0 for success
 * SideEffect:  set sampling frequency
 * 
 */
int8_t Set_Sample_Rate(uint16_t frequency){
    Write_DSP(SB_OUTPUT_RATE);
    Write_DSP((uint8_t)((frequency >> 8) & 0xFF));
    Write_DSP((uint8_t)(frequency & 0xFF));
    return 0;
}

/*
 * int8_t play_music(uint8_t* filename)
 * 
 * Description: play the music
 * Input:       filename
 * Output:      0 for success, -1 for failure
 * SideEffect:  play the music
 * 
 */
int8_t play_music(uint8_t* filename){
    if(is_playing == 1){
        return 0;
    }
    dentry_t audio_dentry;
    if (read_dentry_by_name(filename, &audio_dentry) == -1){
        printf("No Such File");
        return -1;
    }else{
        audio_file_inode = audio_dentry.inode;
    }
    uint8_t magic[4];
    read_data(audio_file_inode, 0, magic, 4);
    if(*((uint32_t*)magic) != RIFF){
        printf("file cannot be played! %s\n", magic);
        return -1;
    }

    Reset_DSP();
    strncpy(audio_filename, (int8_t*)filename, strlen((int8_t*)filename));

    int32_t fd = open(filename);
    _fd = fd;
    uint32_t bytes_read = read_data(audio_file_inode,current_offset, (uint8_t*)DMA_Buffer, BLK_SIZE*2);
    Program_DMA_Controller(1, 0x48 | 0x10, (uint32_t)(&DMA_Buffer[0]), sizeof(DMA_Buffer));
    Set_Sample_Rate(8000);
    start_play(BLK_SIZE);
    is_playing = 1;
    current_offset += bytes_read;

    return 0;
}

/*
 * void start_play(uint32_t block_size)
 * 
 * Description: play the music part
 * Input:       music part size
 * Output:      None
 * SideEffect:  play the music part
 * 
 */
void start_play(uint32_t block_size){
    uint16_t blksize = block_size;
    uint8_t cmd = 0;
    uint8_t mode = 0;
    cmd |= Start_8_bit;
    cmd |= DSP_Reset;
    mode |= 0x00;
    Write_DSP(cmd);
    Write_DSP(mode);
    blksize--;
    Write_DSP((uint8_t)(blksize & 0xFF));
    Write_DSP((uint8_t)((blksize >> 8) & 0xFF));
}

/*
 * void stop()
 * 
 * Description: stop the music
 * Input:       None
 * Output:      None
 * SideEffect:  stop the music
 * 
 */
void stop(){
    uint8_t i = 0;
    Write_DSP(Pause_8_bit);
    is_playing = 0;
    audio_file_inode = 0;
    current_offset = 0;
    cur_block = block1;
    for(i = 0; i < 33; i ++){
        audio_filename[i] = 0;
    }
}

/*
 * void sb16_handler()
 * 
 * Description: handle sb16 interrupt
 * Input:       None
 * Output:      None
 * SideEffect:  check the state and play the music
 * 
 */
void sb16_handler(){
    DSP_inb(0xE);
    send_eoi(5);
    cli();
    uint32_t bytes_read = read_data(audio_file_inode, current_offset, (uint8_t*)cur_block, BLK_SIZE);
    current_offset += bytes_read;
    sti();

    if(bytes_read == BLK_SIZE){
        uint16_t blksize = (uint16_t) bytes_read;
        if(cur_block == block1){
            cur_block = block2;
        }else{
            cur_block = block1;
        }
        start_play(blksize);
    }else if(bytes_read == 0){
        stop();
    }
    return;
}

