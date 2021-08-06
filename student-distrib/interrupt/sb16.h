#include "../library/lib.h"
#include "sys_call.h"
#include "../filesys.h"

#ifndef _DB16_H
#define _DB16_H

#define DSP_Reset               0x6
#define DSP_Read                0xA
#define DSP_Write               0xC
#define DSP_Read_Buffer_Status  0xE
#define SB16_IOBase             0x220
#define SB_OUTPUT_RATE          0x41

#define DMA_1_MASK              0x0A
#define DMA_1_MODE              0x0B
#define DMA_1_CLEAR_PTR         0x0C
#define DMA_1_BASE              0x00

#define DMA_BIT_2               (1 << 2)

#define RIFF                    0x46464952

#define Pause_8_bit             0xD0
#define Start_8_bit             0xC0


int8_t Reset_DSP();
int8_t Write_DSP(uint8_t data);
uint8_t Read_From_DSP();
int8_t Transfer_Sound_DMA(uint8_t channel, uint8_t mode, uint32_t addr, uint32_t size);
int8_t Set_Sample_Rate(uint16_t frequency);
void sb16_handler();
int8_t play_music(uint8_t* filename);
void start_play(uint32_t block_size);
void stop();

#endif
