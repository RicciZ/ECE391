#ifndef SPEAKER_H
#define SPEAKER_H

#include "library/lib.h"
#include "interrupt/sys_call.h"
#include "interrupt/pit.h"
#include "interrupt/rtc.h"

#define SPEAKER_INIT 0xb6
#define SPEAKER_PORT_DATA 0x42
#define SPEAKER_PORT_CMD 0x43
#define SPEAKER_PORT_PLAY 0x61
#define SPEAKER_MASK_MUTE 0xFC
#define SPEAKER_MASK_UNMUTE 0x03

// Make a beep
void beep(uint32_t freq);

#endif
