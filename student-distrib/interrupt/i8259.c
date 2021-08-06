/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "../library/lib.h"
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = MASK; /* IRQs 0-7  */
uint8_t slave_mask = MASK;  /* IRQs 8-15 */

/* 
 * i8259_init
 *  DESCRIPTION: Initialize the 8259 PIC
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Initialize the 8259 PIC, connect master PIC with slave PIC
 */
void i8259_init(void) {
    /* maske all interrupts of PIC*/
    outb(MASK, MASTER_8259_DATA); 
    outb(MASK, SLAVE_8259_DATA);    

    /*Initialization Sequence */
    outb(ICW1,MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);

    outb(ICW1,SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);              
    outb(ICW3_SLAVE, SLAVE_8259_DATA);              
    outb(ICW4, SLAVE_8259_DATA);

    /* Restore IRQ mask*/
    outb(master_mask, MASTER_8259_DATA); 
    outb(slave_mask, SLAVE_8259_DATA); 

    /* connect master PIC with slave PIC*/
    enable_irq(SLAVE_PIN);
}

/* 
 * enable_irq
 *  DESCRIPTION: Enable (unmask) the specified IRQ 
 *  INPUTS: irq_num -- specified IRQ number
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Enable the specified IRQ 
 */

void enable_irq(uint32_t irq_num) {
    /* specified IRQ is on master PIC */
    if( irq_num < SLAVE_OFFSET){
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    /* specified IRQ is on slave PIC */
    }else if(irq_num < TOTAL_NUMBER){
        irq_num -= SLAVE_OFFSET;
        slave_mask &= ~(1 << irq_num);
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* 
 * disable_irq
 *  DESCRIPTION: Disable (unmask) the specified IRQ 
 *  INPUTS: irq_num -- specified IRQ number
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Disable the specified IRQ 
 */
void disable_irq(uint32_t irq_num) {
     /* specified IRQ is on master PIC */
    if( irq_num < SLAVE_OFFSET){
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    /* specified IRQ is on slave PIC */
    }else if(irq_num < TOTAL_NUMBER){
        irq_num -= SLAVE_OFFSET;
        slave_mask |= (1 << irq_num);
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* 
 * send_eoi
 *  DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *  INPUTS: irq_num -- specified IRQ number
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: Send end-of-interrupt signal for the specified IRQ
 */
void send_eoi(uint32_t irq_num) {
    /*the IRQ comes from master PIC*/
    if(irq_num < SLAVE_OFFSET){
        outb(EOI + irq_num, MASTER_8259_PORT);

    /*the IRQ comes from slave PIC*/
    }else if(irq_num < TOTAL_NUMBER){
        outb(EOI + (irq_num - SLAVE_OFFSET), SLAVE_8259_PORT);
        outb(EOI + SLAVE_PIN, MASTER_8259_PORT);
    }
}
