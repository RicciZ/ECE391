#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "sys_call.h"
#include "../process.h"
#include "../library/lib.h"

/*
*	function name:	RAISE_EXCEPTION
*	description:	print the exception message, halt the current process,
*					and return 256 (as the return value of system_execute).
*	inputs:			none
*	outputs:		return 256
*/
#define RAISE_EXCEPTION(exception_name,msg)  	\
void exception_name() {							\
	clear();									\
    printf("\n****** %s ******\n\n", #msg);		\
    printf("Press r to return.\n");				\
    exception_handled = -1;						\
	display_terminal->flag_function = 2;		\
    while (exception_handled == -1){};			\
	display_terminal->flag_function = 0;		\
    extern int32_t running_process;				\
	PCB_t* ptr = get_PCB(running_process);		\
	if (ptr != NULL){							\
		ptr->flag_exception = 1;				\
	}											\
	halt(0);									\
}										        \

#define SYSCALL 	0x80
#define RTC 		0x28
#define KEYBOARD 	0x21
#define PIT			0x20

volatile int32_t exception_handled;

extern void init_interrupt(void);

#endif
