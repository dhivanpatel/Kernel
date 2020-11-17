#define _EXCEPTIONS_H   1
#define EXCEPT_HANDL_H  1

#include "except_handl.h"
#include "exceptions.h"

# Assembly linkage code for going to exception handlers

//Preprocessor macro to simplify exception handling

#define EXCEPT(handl_func) \
pushal; \
pushfl; \
call handl_func; \
popfl; \
popal; \
iret;


//make the exception linkages global
.globl exc_div_by_zero, exc_debug, exc_nmi, exc_break
.globl exc_overflow, exc_bound_range, exc_invalid_op,
.globl exc_dev_not_available, exc_coprocessor
.globl exc_double_fault, exc_invalid_tss, exc_seg_not_pres
.globl exc_stack_seg_fault, exc_gp_fault, exc_sec_exc
.globl exc_virt_exc, exc_align_check, exc_machine_check
.globl exc_page_fault, exc_87_fp_exc, exc_simd_fp_exc



//the linkers
exc_div_by_zero:
  pushal
  pushfl
  call handle_divide_by_zero
  popfl
  popal
  iret
exc_debug:
    EXCEPT(handle_debug)
exc_nmi:
    EXCEPT(handle_nmi)
exc_break:
    EXCEPT(handle_breakpoint)
exc_overflow:
    EXCEPT(handle_overflow)
exc_bound_range:
    EXCEPT(handle_bound_range_exc)
exc_invalid_op:
    EXCEPT(handle_inv_opcode)
exc_dev_not_available:
    EXCEPT(handle_dev_not_available)
exc_double_fault:
    EXCEPT(handle_double_fault)
exc_coprocessor:
    EXCEPT(handle_cop)
exc_invalid_tss:
    EXCEPT(handle_invalid_tss)
exc_seg_not_pres:
    EXCEPT(handle_seg_not_pres)
exc_stack_seg_fault:
    EXCEPT(handle_stack_seg_fault)
exc_gp_fault:
    EXCEPT(handle_gp_fault)
exc_page_fault:
    EXCEPT(handle_page_fault)

exc_87_fp_exc:
    EXCEPT(handle_87_fp_exc)
exc_align_check:
    EXCEPT(handle_align_check)
exc_machine_check:
    EXCEPT(handle_machine_check)

exc_simd_fp_exc:
    EXCEPT(handle_SIMD_fp_exc)
exc_virt_exc:
    EXCEPT(handle_virt_exc)

exc_sec_exc:
    EXCEPT(handle_sec_exc)
