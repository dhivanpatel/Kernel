#include "setup_idt.h"


#define TESTING

/* setup_idt
 * sets up interrupt descriptor table with appropriate exceptions
 * INPUTS: None
 * OUTPUTS: None
 * RETURN VALUE: None
 * SIDE EFFECTS: modifies IDT
 */
void setup_idt()
{
  //first set of interrupt handlers are as follows:
  //exceptions
  //we want assembly linkage for these to???
  uint32_t i;
  for(i = 0; i < NUM_VEC; i++)
  {
    //initialize each idt with some
    //basic stuff
    //wanna make a dummy one?
    /*Set validity based on vector number*/
    idt[i].seg_selector = KERNEL_CS;
    if((i < MASTER_8259_PORT) || (i == KEYBOARD_INTERRUPT + MASTER_8259_PORT) || (i == RTC_INTERRUPT + MASTER_8259_PORT) || (i == SYSCALL_VEC) || (i == PIT_VEC))
    {
      idt[i].present = 1;
    }
    else
    {
      idt[i].present = 0;
    }

    /*Set up privilege levels accordingly*/
    /*Also decide on trap or interrupt gate*/
    if(i == SYSCALL_VEC)
    {
      idt[i].dpl = 3;
    }
    else
    {
      idt[i].dpl = 0;
    }

    //idt[i].dpl = 0;
    /*Set other bits*/
    idt[i].reserved0 = 0;
    idt[i].size = 1;
    idt[i].reserved1 = 1;
    idt[i].reserved2 = 1;
    idt[i].reserved3 = 0;
  }

  //set the idt entries for the exceptions
  SET_IDT_ENTRY(idt[DIVIDE_BY_ZERO], exc_div_by_zero);
  SET_IDT_ENTRY(idt[DEBUG], exc_debug);
  SET_IDT_ENTRY(idt[NMI], exc_nmi);
  SET_IDT_ENTRY(idt[BREAKPOINT], exc_break);
  SET_IDT_ENTRY(idt[OVERFLOW], exc_overflow);
  SET_IDT_ENTRY(idt[BOUND_RANGE], exc_bound_range);
  SET_IDT_ENTRY(idt[INVALID_OPC], exc_invalid_op);
  SET_IDT_ENTRY(idt[DEVICE_NOT_AVAILABLE], exc_dev_not_available);
  SET_IDT_ENTRY(idt[DOUBLE_FAULT], exc_double_fault);
  SET_IDT_ENTRY(idt[COPROCESSOR], exc_coprocessor);
  SET_IDT_ENTRY(idt[INV_TSS], exc_invalid_tss);
  SET_IDT_ENTRY(idt[SEG_NOT_PRES], exc_seg_not_pres);
  SET_IDT_ENTRY(idt[STACK_SEG_FAULT], exc_stack_seg_fault);
  SET_IDT_ENTRY(idt[PAGE_FAULT], exc_page_fault);
  SET_IDT_ENTRY(idt[GP_FAULT], exc_gp_fault);
  SET_IDT_ENTRY(idt[FP_EXC], exc_87_fp_exc);
  SET_IDT_ENTRY(idt[ALIGN_CHECK], exc_align_check);
  SET_IDT_ENTRY(idt[MACHINE_CHECK], exc_machine_check);
  SET_IDT_ENTRY(idt[SIMD_FP_EXC], exc_simd_fp_exc);
  SET_IDT_ENTRY(idt[SEC_EXC], exc_sec_exc);
  SET_IDT_ENTRY(idt[VIRT_EXC], exc_virt_exc);

  SET_IDT_ENTRY(idt[PIT_VEC] ,  &handle_pit_interrupt);
  SET_IDT_ENTRY(idt[SYSCALL_VEC], &handle_syscalls);




  //set idt entries for the keyboard and the clock
  #ifdef TESTING
  SET_IDT_ENTRY(idt[KEYBOARD_VEC], &test_handle_kb_interrupt);
  SET_IDT_ENTRY(idt[RTC_VEC], &test_handle_rtc_interrupt);
  #endif

  #ifndef TESTING
  SET_IDT_ENTRY(idt[KEYBOARD_VEC], &handle_kb_interrupt);
  SET_IDT_ENTRY(idt[RTC_VEC], &handle_rtc_interrupt);
  #endif

}
