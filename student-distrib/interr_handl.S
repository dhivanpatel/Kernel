#define INTERR_HANDL_H 1
#define ASM 1
#include "interr_handl.h"





  // Interrupt handler linkage function
  // an interrupt value will be stored in the stack Im guessing
  // that will have the return address as well as the



.data
    ARG_LOC  = 0        //declared those offsets form the header file here
    ARG_SIZE = 4        // wasnt too sure where else
    SYS_ARG_SIZE = 12
    TEST_KB  = 0x21
    TEST_RTC = 0x28
    KEYBOARD_INTERRUPT = 1
    RTC_INTERRUPT = 8
    SYSCALL_MIN = 1
    SYSCALL_MAX = 10
    PIT = 0x20





.globl handle_kb_interrupt, test_handle_kb_interrupt
.globl handle_rtc_interrupt, test_handle_rtc_interrupt
.globl handle_syscalls, handle_pit_interrupt


syscall_table:
  .long 0x0     # need this here because syscalls go from 1-10 so the zeroth spot takes a garbage value
  .long halt
  .long execute
  .long read
  .long write
  .long open
  .long close
  .long getargs
  .long vidmap
  .long set_handler
  .long sig_return


handle_syscalls:
    sti
    pushl %ebp
    pushl %esi
    pushl %edi # push all except eax
    pushfl    # pushed on flags
    //cli

    //sti
    # must push on arguments for the sys call

    pushl %edx
    pushl %ecx
    pushl %ebx

    # %eax contains the sys call number

    cmpl $SYSCALL_MAX, %eax
    ja bad_call
    cmpl $SYSCALL_MIN, %eax
    jb bad_call


    call *syscall_table(,%eax,4) # jump to the correct function system call
    jmp return_to_wrapper

bad_call:
    movl $-1, %eax
return_to_wrapper:
    popl %ebx
    popl %ecx
    popl %edx
    # above we first pop off the args
    //sti
    cli
    popfl
    popl %edi
    popl %esi
    popl %ebp
    iret



//////////////////////////////////////////////////

handle_pit_interrupt:
    pushl $PIT
    jmp handle_PIC_interrupt
handle_kb_interrupt:
    //sti
    pushl $KEYBOARD_INTERRUPT
    jmp handle_PIC_interrupt

handle_rtc_interrupt:
    //sti
    pushl $RTC_INTERRUPT
    jmp handle_PIC_interrupt

# These two functions are used for testing
test_handle_kb_interrupt:
    //sti
    pushl $TEST_KB
    jmp handle_PIC_interrupt

test_handle_rtc_interrupt:
    //sti
    pushl $TEST_RTC
    jmp handle_PIC_interrupt

handle_PIC_interrupt:
    # need to get the irq number from the array
    sti
    movl ARG_LOC(%esp), %eax
    pushal
    pushfl
    cmpl $TEST_RTC, %eax
    jne test_for_kb
    call test_handle_rtc
    jmp ret_from_handler
test_for_kb:
    cmpl $TEST_KB, %eax
    jne check_for_rtc
    call test_handle_keyboard
    jmp ret_from_handler
check_for_rtc:
    cmpl $RTC_INTERRUPT, %eax
    jne check_for_kb
    call handle_rtc
    jmp ret_from_handler
check_for_kb:
    cmpl $KEYBOARD_INTERRUPT, %eax
    jne check_for_pit
    call handle_keyboard
    jmp ret_from_handler
check_for_pit:
    cmpl $PIT, %eax
    jne ret_from_handler
    call schedule           # every time pit generates interrupts new process scheduled
ret_from_handler:
    cli
    popfl
    popal
    addl $ARG_SIZE, %esp # remove last argument
    iret
