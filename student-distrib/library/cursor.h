/*
*   This file deals with all the operation related to cursor
*/

#ifndef CURSOR_H
#define CURSOR_H

#include "lib.h"

#define CURSOR_CMD  0x3D4
#define CURSOR_DATA 0x3D5

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);
void switch_cursor(int x, int y);
uint16_t get_cursor_position(void);

#endif
