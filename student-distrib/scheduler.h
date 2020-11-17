#ifndef SCHED_H
#define SCHED_H

#include "types.h"
#include "i8259.h"
#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"
#include "PCB.h"
#include "syshelp.h"
#include "keyboard.h"

#define PIT_IRQ       0   // the programmable interval timer is set at IRQ 0 of the PIC

#define COMMAND_REG   0x43

#define INPUT_CLOCK   1193180

#define SQUARE_WAVE   0x36

#define CHANNEL_ZERO   0x40

#define EIGHT_MB      0x800000

#define EIGHT_KB      8192


/*REFERENCES
  http://www.osdever.net/bkerndev/Docs/pit.htm
  https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial
  https://wiki.osdev.org/Programmable_Interval_Timer
  https://wiki.osdev.org/Scheduling_Algorithms
 */




//on context switch, switch pd


extern void handle_pit_interrupt(void);

void pit_init(/*int hz*/);

void schedule();






#endif
