#include "keyboard.h"
#include "i8259.h"
#include "../library/lib.h"
#include "../terminal.h"
#include "../process.h"
#include "idt_init.h"



//check if specific key is pressed
typedef enum {false, true} bool;
bool Shift_Pressed = false;
bool Cap_Pressed = false;
bool Alt_Pressed = false;
bool Ctrl_Pressed = false;
uint8_t temp_buf[BUFFER_SIZE];
int32_t halt_terminal = -1;

/* Table for scan set code, lowercase and released shift*/
unsigned char scanCodeSet_low[SCANCODE_NUM] = {
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8',
  '9', '0', '-', '=', '\b',	'\t', 'q', 'w', 'e', 'r',	
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,			
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	
 '\'', '`',   0,	'\\', 'z', 'x', 'c', 'v', 'b', 'n',	
  'm', ',', '.', '/',   0,	'*', 0, ' '
};

/* Table for scan set code, uppercase and released shift*/
unsigned char scanCodeSet_high[SCANCODE_NUM] = {
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8',
  '9', '0', '-', '=', '\b',	'\t', 'Q', 'W', 'E', 'R',	
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0,			
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	
 '\'', '`',   0,	'\\', 'Z', 'X', 'C', 'V', 'B', 'N',	
  'M', ',', '.', '/',   0,	'*', 0, ' '
};

/* Table for scan set code, uppercase and pressed shift*/
unsigned char scanCodeSet_high_shift[SCANCODE_NUM] = {
    0,  0, '!', '@', '#', '$', '%', '^', '&', '*',
  '(', ')', '_', '+', '\b',	'\t', 'q', 'w', 'e', 'r',	
  't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', 0,			
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',	
 '"', '~',   0,	'|', 'z', 'x', 'c', 'v', 'b', 'n',	
  'm', '<', '>', '?',   0,	'*', 0, ' '
};

/* Table for scan set code, lowercase and pressed shift*/
unsigned char scanCodeSet_low_shift[SCANCODE_NUM] = {
    0,  0, '!', '@', '#', '$', '%', '^', '&', '*',
  '(', ')', '_', '+', '\b',	'\t', 'Q', 'W', 'E', 'R',	
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,			
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	
 '"', '~',   0,	'|', 'Z', 'X', 'C', 'V', 'B', 'N',	
  'M', '<', '>', '?',   0,	'*', 0, ' '
};






/* 
 * keyboard_init
 *  DESCRIPTION: initialize keyboard
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable keyboard IRQ 
 */
void keyboard_init(void){
    enable_irq(KEYBOARD_IRQ);
    display_terminal->flag_function = 0;
}



/* 
 * keyboard_interrupt
 *  DESCRIPTION: handle keyboard interrupt
 *  INPUTS: none
 *  OUTPUTS: print the pressed keyboard
 *  RETURN VALUE: none
 *  SIDE EFFECTS: read the pressed key, print in ASCII
 *                 send eoi to keyboard port  
 */
void keyboard_interrupt(void){
    cli();
    scanCode = inb(KEYBOARD_DATA_PORT);
    //handler specicific scancode
    switch (scanCode)
    {
    //special scancode
    case CAP_P_SC:
        Cap_Pressed = ~Cap_Pressed;
        break;
    case LSHIFT_P_SC: 
    case RSHIFT_P_SC:
        Shift_Pressed = true;
        break;
    case LSHIFT_R_SC: 
    case RSHIFT_R_SC:
        Shift_Pressed = false;
        break;
    case ALT_P_SC: 
        Alt_Pressed = true;
        break;
    case ALT_R_SC:
        Alt_Pressed = false;
        break;
    case CTRL_P_SC:
        Ctrl_Pressed = true;
        break;
    case CTRL_R_SC:
        Ctrl_Pressed = false;
        break;
    case L_SC: // clear the screen
        if (display_terminal->flag_function == 0 || display_terminal->flag_function == 1){  // display in a terminal or keyboard buffering
            if(Ctrl_Pressed){
                clear();
                register int32_t tmp_id = 0;
                while (display_terminal->keyboard_buf[tmp_id] != '\0') {
                    putc_visible(display_terminal->keyboard_buf[tmp_id]);
                    tmp_id++;
                }
                break;
            }
            else if (Alt_Pressed){
                clear_history();
                break;
            }
            else{
                normal_key(scanCode);
                break;
            }
        }
    case C_SC:  // halt the current process
        if (Ctrl_Pressed){
            halt_terminal = display_terminal->tid;
            break;
        }
        else{
            if (display_terminal->flag_function == 0 || display_terminal->flag_function == 1){
                normal_key(scanCode);
                break;
            }
        }
    case F1_SC:
        if(Alt_Pressed){
            terminal_switch(0);
        }
        break;
    case F2_SC:
        if(Alt_Pressed){
            terminal_switch(1);
        }
        break;
    case F3_SC:
        if(Alt_Pressed){
            terminal_switch(2);
        }
        break;
    case UP:
        search_history(1);
        break;
    case DOWN:
        search_history(0);
        break;

    //normal scancode   
    default:
        if (display_terminal->flag_function == 0 || display_terminal->flag_function == 1){  // display in a terminal or keyboard buffering
            normal_key(scanCode);
        }
        else if (display_terminal->flag_function == 2){  // an exception is thrown
            if (scanCode == R_SC){
                if (exception_handled == -1){       
                    putc_visible('r');
                    putc_visible('\n');
                    exception_handled = 1;
                    break;
                }
            }
        }
    }
    send_eoi(KEYBOARD_IRQ);
    sti();
}



/* 
 * normal_key
 *  DESCRIPTION: helper function for handling normal input key
 *  INPUTS: scanCode -- the scanCode of pressed key
 *  OUTPUTS: print the pressed keyboard
 *  RETURN VALUE: none
 *  SIDE EFFECTS: store the pressed key into keyboard buffer
 */
void normal_key(uint8_t scanCode){
    unsigned char key_print;

    //judge the current scanCode mode
    if ((scanCode <= SCANCODE_NUM) && (display_terminal->read_count < BUFFER_SIZE - 1)){// last two char for '\n'
        if (Cap_Pressed){
            if(Shift_Pressed){
                key_print = scanCodeSet_high_shift[scanCode];
            }else{
                key_print = scanCodeSet_high[scanCode];
            }
        }else{
            if(Shift_Pressed){
                key_print = scanCodeSet_low_shift[scanCode];
            }else{
                key_print = scanCodeSet_low[scanCode];
            }
        }
    //After 127 char input, delete or enter is still avaliable
    } else if ((scanCode == BACKSPACE_SC) || (scanCode == ENTER_P_SC)){
        key_print = scanCodeSet_high[scanCode];
    }
    else{
        return;
    }

    //handle specific character
    switch (key_print)
    {
    case '\b':
        if(display_terminal->read_count > 0){
            deletec(0);
            display_terminal->keyboard_buf[display_terminal->read_count] = 0;
            display_terminal->read_count -= 1;
        }
        break;
    case '\t':
        tab_pressed();
        break;
    case '\n':
        putc_visible(key_print);
        if (display_terminal->flag_function == 0){
            display_terminal->read_count = 0;
            clear_keyboard_buffer();    // clear the keyboard_buffer
        }
        else if (display_terminal->flag_function == 1){
            display_terminal->keyboard_buf[display_terminal->read_count] = '\n';
            display_terminal->read_count = 0;
            display_terminal->input_done = 1;
        }
        break;
    case 0:
        break;
    default:
        display_terminal->keyboard_buf[display_terminal->read_count] = key_print;
        putc_visible(key_print);
        display_terminal->read_count += 1;
        break;
    }
}



/* 
 * clear_keyboard_buffer
 *  DESCRIPTION: clear the keyboard buffer when enter pressed
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: clear keyboard buffer
 */
void clear_keyboard_buffer(void){
    int i = 0;
    for(;i < BUFFER_SIZE; i++){
        display_terminal->keyboard_buf[i] = 0;
    }
}



/* 
 * tab_pressed
 *  DESCRIPTION: handle tab pressed
 *  INPUTS: none
 *  OUTPUTS: print four space on the screen and store four space into keyboard buffer
 *  RETURN VALUE: none
 *  SIDE EFFECTS: print four space on the screen and four space into keyboard buffer
 */
void tab_pressed(void){
    if (display_terminal->read_count == 0 || display_terminal->keyboard_buf[display_terminal->read_count-1] == ' ')
    {
        // int i = 0;
        // //print four space when tab pressed
        // for (; i < 4; i++)
        // {
        //     display_terminal->keyboard_buf[display_terminal->read_count] = ' ';
        //     putc_visible(' ');
        //     display_terminal->read_count += 1;
        // }
        return;
    }
    else{
        // store the pre-part of the file name
        uint8_t pre[BUFFER_SIZE];
        uint8_t temp[BUFFER_SIZE];
        int i = display_terminal->read_count-1;
        int j = 0;
        while (i >= 0 && display_terminal->keyboard_buf[i] != ' ') temp[j++] = display_terminal->keyboard_buf[i--];
        j--;
        i = 0;
        while (j >= 0) pre[i++] = temp[j--];
        pre[i] = '\0';
        int match_len = i;

        extern boot_blk_t* boot_blk_ptr;
        dentry_t den;
        int match = 0;
        int match_count = 0;
        int match_i = 0;

        // compare with each file name
        for (i = 0; i < boot_blk_ptr->num_dentries; i++){
            // store file name in temp
            if (read_dentry_by_index(i, &den)) return;
            strncpy((int8_t*)temp, den.name, MAX_FILENAME_LEN);
            ((char*)temp)[MAX_FILENAME_LEN] = '\0';
            int32_t len = (int32_t)strlen((int8_t*)temp);
            ((char*)temp)[len] = '\0';

            int match_flag = 1;
            for (j = 0; pre[j] != '\0'; j++){
                if (pre[j] != temp[j]){
                    match_flag = 0;
                    break;
                }
            }
            if (match_flag){
                match = 1;
                match_i = i;
                match_count++;
            }
            if (match_count > 1) return;
        }

        // if match, print it
        if (match){
            read_dentry_by_index(match_i, &den);
            strncpy((int8_t*)temp, den.name, MAX_FILENAME_LEN);
            ((char*)temp)[MAX_FILENAME_LEN] = '\0';
            int32_t len = (int32_t)strlen((int8_t*)temp);
            ((char*)temp)[len] = '\0';
            for (i = 0, j = 0; temp[j] != '\0'; i++,j++){
                if (i < match_len) continue;
                display_terminal->keyboard_buf[display_terminal->read_count] = temp[j];
                putc_visible(temp[j]);
                display_terminal->read_count++;
            }
        }
    }    
}
