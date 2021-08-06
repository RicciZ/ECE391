#include "rtc.h"

volatile uint16_t x_int[TERMINAL_NUM] = {1,1,1};            // x interrupts raise a real interrupt
volatile uint16_t int_count[TERMINAL_NUM] = {0,0,0};        // interrupt count
static volatile uint32_t time = 0;                          // count total time
extern fd_t* fd_array;

/* rtc_init
 * 
 * Initialize the RTC.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void rtc_init(void) {
    cli();
    outb(RTC_REG_B, PORT_ID);	    // select register B, and disable NMI
    char prev = inb(PORT_RW);	    // read the current value of register B
    outb(RTC_REG_B, PORT_ID);	    // select register B, and disable NMI
    outb(prev | 0x40, PORT_RW);     // write the prevRWus value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(IRQ8);

    // set the RTC frequency to 1024 Hz
    outb(RTC_REG_A, PORT_ID);       // select register A, and disable NMI
    prev = inb(PORT_RW);            // get init value of register A
    outb(RTC_REG_A, PORT_ID);       // select register A, and disable NMI
    outb((prev & 0xF0) | SET_FREQ_1024, PORT_RW);   // write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
}

/* rtc_interrupt
 * 
 * Keep sending RTC interrupt.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void rtc_interrupt(void) {
    cli();
    int i;
    for (i = 0; i < TERMINAL_NUM; i++){
        if (int_count[i] > 0) int_count[i]--;
    }
    time++;
    outb(RTC_REG_C, PORT_ID);       // select register C
    inb(PORT_RW);		            // just throw away contents
    send_eoi(IRQ8);
    sti();
}


/* rtc_open
 * 
 * Set RTC to 2 Hz.
 * Inputs: Ignore the input.
 * Outputs: 0 for success.
 * Side Effects: None
 */
int32_t rtc_open (const uint8_t* filename) {
    x_int[running_terminal->tid] = FREQ_1024 / FREQ_2;     // set freq to 2, i.e. x_int = 512
    int_count[running_terminal->tid] = x_int[running_terminal->tid];
    return 0;
}


/* rtc_close
 * 
 * Reset RTC to 1024 Hz.
 * Inputs: Ignore the input.
 * Outputs: 0 for success.
 * Side Effects: None
 */
int32_t rtc_close (int32_t fd) {
    x_int[running_terminal->tid] = 1;
    int_count[running_terminal->tid] = 0;
    return 0;
}


/* rtc_read
 * 
 * Wait for the real interrupt.
 * Inputs: Ignore the input.
 * Outputs: 0 for success.
 * Side Effects: None
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes) {
    while (int_count[running_terminal->tid] > 0);
    int_count[running_terminal->tid] = x_int[running_terminal->tid];
    return 0;
}


/* rtc_write
 * 
 * Set the frequency to user input frequency if valid.
 * Inputs: buf - the int32_t pointer to the frequency we want to set
 * Outputs: 0 for success and -1 for failure.
 * Side Effects: None
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes) {
    if (!buf) return -1;
    
    int32_t freq = *(int32_t *)buf;
    if (freq > 1024 || freq < 2 || ((freq & (freq-1)) != 0))
        return -1;
    x_int[running_terminal->tid] = FREQ_1024 / freq;
    return 0;
}


/* rand
 * 
 * An RTC based random number generator
 * Inputs: None
 * Outputs: the random number
 * Side Effects: None 
 */
int rand() // RAND_MAX assumed to be 32767
{
    time = time * 1103515245 + 12345;
    return (unsigned int)(time / 65536) % 32768;
}

void timer_wait(int32_t time){
    int i,garbage;
    fd_t tmp_fd_array[MAX_FILES];
    init_fd(tmp_fd_array);
    int32_t fd = open((uint8_t*)"rtc");         // 2Hz
    for (i = 0; i < time; i++) read(fd,&garbage,0);
    close(fd);
}
