#ifndef IDT_LINKAGE_H
#define IDT_LINKAGE_H

extern void system_call(void);

extern void rtc_handler(void);

extern void keyboard_handler(void);

extern void pit_handler(void);

#endif
