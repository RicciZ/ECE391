#ifndef _PIT_H
#define _PIT_H

#include "../types.h"
#include "../library/lib.h"
#include "../process.h"

#define PIT_DIVIDEND    1193180     //Hz, input clock

#define MODE3               0x36
#define CHANNEL0_DATA_PORT  0x40    //Channel 0 data port (read/write)
#define CMD_REG             0x43    //Mode/Command register (write only, a read is ignored)
#define FRE                 100     //setting to 100Hz
#define IRQ0                0x0      

void pit_init();
void pit_interrupt();
void pit_phase(uint32_t hz);

#endif
