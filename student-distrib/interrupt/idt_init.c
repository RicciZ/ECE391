#include "../x86_desc.h"
#include "../library/lib.h"
#include "idt_init.h"
#include "idt_linkage.h"

void system_call_handler(void);

RAISE_EXCEPTION(DIVIDE_EXCEPTION,"Divide Error");
RAISE_EXCEPTION(DEBUG_EXCEPTION,"Debug Exception");
RAISE_EXCEPTION(NMI_EXCEPTION,"Non Maskable Interrupt Exception");
RAISE_EXCEPTION(INT3_EXCEPTION,"Breakpoint Exception");
RAISE_EXCEPTION(OVERFLOW_EXCEPTION,"Overflow Exception");
RAISE_EXCEPTION(BOUNDS_EXCEPTION,"BOUND Range Exceeded Exception");
RAISE_EXCEPTION(INVALID_OPCODE_EXCEPTION,"Invalid Opcode Exception");
RAISE_EXCEPTION(DEVICE_NOT_AVAILABLE_EXCEPTION,"Device Not Available Exception");
RAISE_EXCEPTION(DOUBLE_FAULT_EXCEPTION,"Double Fault Exception");
RAISE_EXCEPTION(COPROCESSOR_SEGMENT_OVERRUN_EXCEPTION,"Coprocessor Segment Exception");
RAISE_EXCEPTION(TSS_EXCEPTION,"Invalid TSS Exception");
RAISE_EXCEPTION(SEG_NOT_PRESENT_EXCEPTION,"Segment Not Present");
RAISE_EXCEPTION(STACK_SEGMENT_EXCEPTION,"Stack Fault Exception");
RAISE_EXCEPTION(GENERAL_PROTECTION_EXCEPTION,"General Protection Exception");
RAISE_EXCEPTION(PAGE_FAULT_EXCEPTION,"Page Fault Exception");
RAISE_EXCEPTION(FLOAT_EXCEPTION,"Floating Point Exception");
RAISE_EXCEPTION(ALIGN_CHECK_EXCEPTION,"Alignment Check Exception");
RAISE_EXCEPTION(MACHINE_CHECK_EXCEPTION,"Machine Check Exception");
RAISE_EXCEPTION(SIMD_FLOATING_POINT_EXCEPTION,"SIMD Floating Point Exception");


/* init_interrupt
 * 
 * Initialize the IDT.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void init_interrupt(void){
    int i = 0;
    for (; i < NUM_VEC; i++){
        if (i < 32 && i > 19) continue; // 0x14 to 0x1F are not used

        // 0-31 trap gate, 32-255 interrupt gate
        if (i < 32)
            idt[i].reserved3 = 1;
        else{
            idt[i].reserved3 = 0;
        }

        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size      = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl       = 0;
        idt[i].present   = 1;
    }
    SET_IDT_ENTRY(idt[0], DIVIDE_EXCEPTION);
	SET_IDT_ENTRY(idt[1], DEBUG_EXCEPTION);
	SET_IDT_ENTRY(idt[2], NMI_EXCEPTION);
	SET_IDT_ENTRY(idt[3], INT3_EXCEPTION);
	SET_IDT_ENTRY(idt[4], OVERFLOW_EXCEPTION);
	SET_IDT_ENTRY(idt[5], BOUNDS_EXCEPTION);
	SET_IDT_ENTRY(idt[6], INVALID_OPCODE_EXCEPTION);
	SET_IDT_ENTRY(idt[7], DEVICE_NOT_AVAILABLE_EXCEPTION);
	SET_IDT_ENTRY(idt[8], DOUBLE_FAULT_EXCEPTION);
	SET_IDT_ENTRY(idt[9], COPROCESSOR_SEGMENT_OVERRUN_EXCEPTION);
	SET_IDT_ENTRY(idt[10], TSS_EXCEPTION);
	SET_IDT_ENTRY(idt[11], SEG_NOT_PRESENT_EXCEPTION);
	SET_IDT_ENTRY(idt[12], STACK_SEGMENT_EXCEPTION);
	SET_IDT_ENTRY(idt[13], GENERAL_PROTECTION_EXCEPTION);
	SET_IDT_ENTRY(idt[14], PAGE_FAULT_EXCEPTION);
	SET_IDT_ENTRY(idt[16], FLOAT_EXCEPTION);
	SET_IDT_ENTRY(idt[17], ALIGN_CHECK_EXCEPTION);
	SET_IDT_ENTRY(idt[18], MACHINE_CHECK_EXCEPTION);
    SET_IDT_ENTRY(idt[19], SIMD_FLOATING_POINT_EXCEPTION);

    idt[SYSCALL].dpl = 3;
    SET_IDT_ENTRY(idt[SYSCALL], system_call);

    SET_IDT_ENTRY(idt[RTC], rtc_handler);
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_handler);
    SET_IDT_ENTRY(idt[PIT], pit_handler);
}
