#ifndef SETUP_IDT_H
#define SETUP_IDT_H

#include "except_handl.h"
#include "exceptions.h"
#include "keyboard.h"
#include "rtc.h"
#include "i8259.h"
#include "x86_desc.h"
#include "tests.h"
#include "syscall.h"
#include "scheduler.h"



#define PIT_VEC      0x20     // where the pit is mapped to on the IDT


void setup_idt();
#endif
