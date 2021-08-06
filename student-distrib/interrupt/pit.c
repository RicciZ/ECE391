#include "pit.h"


/* 
 * pit_phase
 *  DESCRIPTION: set the frequency
 *  INPUTS: hz
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: set the frequency 
 */
void pit_phase(uint32_t hz){
    int div = PIT_DIVIDEND / hz;
    outb(MODE3, CMD_REG);   
    outb(div & 0xFF, CHANNEL0_DATA_PORT); //Set low byte of divisor
    outb(div >> 8, CHANNEL0_DATA_PORT); //Set high byte of divisor
}



/* 
 * pit_init
 *  DESCRIPTION: initialize pit
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: initialize pit
 */
void pit_init(){
    enable_irq(IRQ0);
    pit_phase(100);
}



/* 
 * pit_interrupt
 *  DESCRIPTION: handle pit interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: handle pit interrupt
 */
void pit_interrupt() {
    cli();
    send_eoi(IRQ0);
    schedule();
    sti();
}
