#define SYS_HELP_H 1
#include "syshelp.h"

#define ASM 1
#include "x86_desc.h"

.data
    EXEC_ADDR = 8
    PCB_EBP = 12
    PARENT_EBP = 4
    CURRENT_EBP = 4
    CURRENT_ESP = 8
    HALT_VAL = 8
    CUR_EBP = 0
    USER_STACK = 0x83FFFF0
    FLAG_MASK = 0x4200

.text
.globl start_exec, ret_to_exec
.globl get_esp, get_ebp
.globl run_next_proc
start_exec:
    // changed user stack to specified as esp ends up storing
    // more than a byte
    // First, update the segment registers
    // We have these segment registers:
    //  - CS, SS, DS, FS, GS
    movl $USER_DS, %ecx
    movw %cx, %ds
    movw %cx, %es

    // save ebp into PCB struct pointer
    cli
    pushl %ebp
    movl %esp, %ebp
    movl PCB_EBP(%ebp), %ecx
    movl %ebp, (%ecx)
    sti

    // push args for iret
    pushl $USER_DS
    // push address of user stack
    pushl $USER_STACK
    // change flags for user mode
    pushf
    popl %ecx
    orl  $FLAG_MASK, %ecx
    pushl %ecx
    // push other stuff for iret
    pushl $USER_CS
    pushl EXEC_ADDR(%ebp)

    iret

ret_to_exec:
    movl HALT_VAL(%esp), %eax
    movl PARENT_EBP(%esp), %ebp
    leave
    ret

run_next_proc:
    movl CURRENT_EBP(%esp), %ebp
    movl CURRENT_ESP(%esp), %esp
  //  leave
    ret


get_esp:
    movl %esp, %eax

    ret

get_ebp:
    movl %ebp, %eax

    ret

// EIP
//
