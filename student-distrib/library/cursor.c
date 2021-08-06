/*
*	This file refers to OSDEV. Thank those guys so much.
*/

#include "cursor.h"
#include "../types.h"
#include "../terminal.h"



/*
 * void enable_cursor ()
 * inputs:          cursor_start and cursor_end, indicating the range of the cursor
 * return value:    None  
 * outputs:         enable the cursor
 * notes:           
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x0A, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((inb(CURSOR_DATA) & 0xC0) | cursor_start, CURSOR_DATA);
 
	outb(0x0B, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((inb(CURSOR_DATA) & 0xE0) | cursor_end, CURSOR_DATA);
}



/*
 * void disable_cursor ()
 * inputs:          None
 * return value:    None  
 * outputs:         disable the cursor
 * notes:           
 */
void disable_cursor()
{
	outb(0x0A, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb(0x20, CURSOR_DATA);	// refer to OSDEV, set the cursor
}



/*
 * void update_cursor ()
 * inputs:          x and y as the coordinate of the current cursor
 * return value:    None  
 * outputs:         display cursor at desired coordinate
 * notes:           
 */
void update_cursor(int x, int y)
{
	if (running_terminal != display_terminal) return;
	uint16_t pos = y * NUM_COLS + x;
 
	outb(0x0F, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((uint8_t) (pos & 0xFF), CURSOR_DATA);
	outb(0x0E, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((uint8_t) ((pos >> 8) & 0xFF), CURSOR_DATA);
}



/*
 * void switch_cursor ()
 * inputs:          x and y as the coordinate of the current cursor
 * return value:    None  
 * outputs:         display cursor at desired coordinate
 * notes:           
 */
void switch_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;
 
	outb(0x0F, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((uint8_t) (pos & 0xFF), CURSOR_DATA);
	outb(0x0E, CURSOR_CMD);		// refer to OSDEV, set the cursor
	outb((uint8_t) ((pos >> 8) & 0xFF), CURSOR_DATA);
}



/*
 * void get_cursor_position ()
 * inputs:          None
 * return value:    the current position of the cursor
 * outputs:         the return value
 * notes:           
 */
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(0x0F, CURSOR_CMD);		// refer to OSDEV, set the cursor
    pos |= inb(CURSOR_DATA);
    outb(0x0E, CURSOR_CMD);
    pos |= ((uint16_t)inb(CURSOR_DATA)) << 8;
    return pos;
}
