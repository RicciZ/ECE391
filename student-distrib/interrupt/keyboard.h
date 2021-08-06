#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "../types.h"

#define KEYBOARD_IRQ        0x1        //the IRQ number of keyboard
#define KEYBOARD_DATA_PORT  0x60       //Data Port
#define SCANCODE_NUM        0x3A       // size of scancode set 

#define BUFFER_SIZE         128
#define BACKSPACE_SC        0x0E        //backspace scancode
#define CAP_P_SC            0x3A        //CapsLock  Pressed scancode
#define LSHIFT_P_SC         0x2A        //left shift preesed scancode
#define RSHIFT_P_SC         0x36        //right shift preesed scancode
#define LSHIFT_R_SC         0xAA        //left shift released scancode
#define RSHIFT_R_SC         0xB6        //right shift released scancode
#define ALT_P_SC            0x38        //ALT pressed scancode
#define ALT_R_SC            0xB8        //ALT released scancode
#define CTRL_P_SC           0x1D        //CONTROL pressed scancode
#define CTRL_R_SC           0x9D        //CONTROL released scancode
#define L_SC                0x26        //L scancode
#define C_SC                0X2E        //C scancode
#define R_SC                0x13        //R scancode
#define ENTER_P_SC          0x1C        //ENTER pressed scancode
#define F1_SC               0x3B        //F1 pressed scancode
#define F2_SC               0x3C        //F2 pressed scancode
#define F3_SC               0x3D        //F3 pressed scancode
#define UP                  0x48
#define DOWN                0x50

/* the scanCode of pressed key */
uint8_t scanCode;


/* helper function for handling normal key, store it into keyboard
*  buffer and print on the screen*/
void normal_key(uint8_t scanCode);

/* clear the keyboard buffer when enter pressed*/
void clear_keyboard_buffer(void);

/*print four space on the screen and store four space into keyboard buffer*/
void tab_pressed(void);
/*initialize keyboard*/
void keyboard_init(void);

/*handle keyboard interrupt*/
void keyboard_interrupt(void);
#endif
