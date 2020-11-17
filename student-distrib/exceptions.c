#include "exceptions.h"
#include "lib.h"
#include "syshelp.h"
#include "PCB.h"


/*Set of exception handlers to handle
 *Defined exceptions not reserved by intel
 *Each handler prints a message, and then squashes
 *whatever user code was running by cauing an infinite
 *loop
 */

void handle_divide_by_zero(void)
{
  printf("Divide by zero!");
  while(1);
}

void handle_debug(void)
{
  printf("Debug!");
  while(1);
}

void handle_nmi(void)
{
  printf("Non-maskable interrupt !");
  while(1);
}

void handle_breakpoint(void)
{
  printf("Breakpoint !");
  uint32_t esp = get_esp();
  PCB_t* cur_pcb = (PCB_t *)(esp & PCB_MASK);
  ret_to_exec(cur_pcb -> parent_ebp, 256);
}

void handle_overflow(void)
{
  printf("Overflow!");
  while(1);
}

void handle_bound_range_exc(void)
{
  printf("Exceeded bound range! ");
  while(1);
}

void handle_inv_opcode(void)
{
  printf("Invalid opcode!");
  while(1);
}

void handle_dev_not_available(void)
{
  printf("Can't find device");
  while(1);
}

void handle_double_fault(void)
{
  printf("Double fault!");
  while(1);
}

void handle_cop(void)
{
  printf("COPROCESSOR error!");
  while(1);
}

void handle_invalid_tss(void)
{
  printf("Invalid task segment selector");
  while(1);
}

void handle_seg_not_pres(void)
{
  printf("Segment not present!");
  uint32_t esp = get_esp();
  PCB_t* cur_pcb = (PCB_t *)(esp & PCB_MASK);
  ret_to_exec(cur_pcb -> parent_ebp, 256);
}

void handle_stack_seg_fault(void)
{
    printf("Segmentation fault stack");
    uint32_t esp = get_esp();
    PCB_t* cur_pcb = (PCB_t *)(esp & PCB_MASK);
    ret_to_exec(cur_pcb -> parent_ebp, 256);
}

void handle_gp_fault(void)
{
  printf("General Protection Fault");
  uint32_t esp = get_esp();
  PCB_t* cur_pcb = (PCB_t *)(esp & PCB_MASK);
  ret_to_exec(cur_pcb -> parent_ebp, 256);
}

void handle_page_fault(void)
{
  printf("Page Fault!");
  uint32_t esp = get_esp();
  PCB_t* cur_pcb = (PCB_t *)(esp & PCB_MASK);
  ret_to_exec(cur_pcb -> parent_ebp, 256);
}

void handle_87_fp_exc(void)
{
  printf("x87 floating point exception!");
  while(1);
}

void handle_align_check(void)
{
  printf("Error in alignment!");
  while(1);
}

void handle_machine_check(void)
{
  printf("Machine check error!");
  while(1);
}

void handle_SIMD_fp_exc(void)
{
  printf("SIMD floating exception!");
  while(1);
}

void handle_virt_exc(void)
{
  printf("virtualization exception");
  while(1);
}

void handle_sec_exc(void)
{
  printf("Security exception... wait what?");
  while(1);
}
