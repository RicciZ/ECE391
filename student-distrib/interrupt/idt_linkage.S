#define ASM 1



/*
*	interrupt handler linkage:
*	save all the relevant registers, call the corresponding function,
*	restore the registers after return.
*/
#define HANDLER(name,func)			         \
.global name							    ;\
name:										;\
	pushal									;\
	pushfl									;\
	call func       						;\
	popfl									;\
	popal									;\
	iret									;\
	
# keyboard_handler: interrupt handler for keyboard interrupts
HANDLER(keyboard_handler, keyboard_interrupt);
# clock_handler: interrupt handler for rtc interrupts
HANDLER(rtc_handler, rtc_interrupt);
# pit_handler: interrupt handler for scheduling
HANDLER(pit_handler, pit_interrupt);



/*
*	system_call linkage:
*	save all the relevant registers, validate %eax, then jump to 
*	corresponding function, restore registers after return from it.
*/
.global system_call
system_call:

	cli
	pushl %ebp
	pushl %edi
	pushl %esi

	pushl %edx
	pushl %ecx
	pushl %ebx
	sti

	cmp $1, %eax
    jl invalid
    cmp $10, %eax
    jg invalid

	call *syscall_jumptable(,%eax,4)
	jmp done

invalid:
	movl $-1, %eax

done:
	popl %ebx
	popl %ecx
	popl %edx

	popl %esi
	popl %edi
	popl %ebp

	iret

syscall_jumptable:
	.long 0x0				# jumptable starts at 1
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn
