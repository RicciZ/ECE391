#ifndef RTC_H
#define RTC_H

#include "i8259.h"
#include "../library/lib.h"
#include "../terminal.h"
#include "../filesys.h"

#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x0C

#define PORT_ID 0x70                // to select register
#define PORT_RW 0x71                // to enable R/W

#define IRQ8 8                      // IRQ for RTC

#define SET_FREQ_1024 0x06          // frequency = 32768 >> (6-1) = 1024

#define FREQ_1024   1024
#define FREQ_2      2

/* Initialize the RTC */
void rtc_init(void);

/* Keep sending RTC interrupt */
void rtc_interrupt(void);

/* Set RTC to 2 Hz */
int32_t rtc_open (const uint8_t* filename);

/* Reset RTC to 1024 Hz */
int32_t rtc_close (int32_t fd);

/* Wait for the real interrupt */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

/* Set the frequency to user input frequency if valid */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);

/* random number generator */
int rand();

/* a wait timer based on RTC */
void timer_wait(int32_t time);

#endif /* RTC_H */
