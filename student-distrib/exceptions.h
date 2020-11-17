#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H


/*List of exception handlers*/
/*For now, they simply output something
 *and spin around*/

/*constants to aid in setting up idt*/
#define DIVIDE_BY_ZERO 0
#define DEBUG 1
#define NMI 2
#define BREAKPOINT 3
#define OVERFLOW 4
#define BOUND_RANGE 5
#define INVALID_OPC 6
#define DEVICE_NOT_AVAILABLE 7
#define DOUBLE_FAULT 8
#define COPROCESSOR 9
#define INV_TSS 10
#define SEG_NOT_PRES 11
#define STACK_SEG_FAULT 12
#define GP_FAULT 13
#define PAGE_FAULT 14

#define FP_EXC 16
#define ALIGN_CHECK 17
#define MACHINE_CHECK 18
#define SIMD_FP_EXC 19
#define VIRT_EXC 20

#define SEC_EXC 30


/*Exceptions 0 through 14*/
extern void handle_divide_by_zero(void);
extern void handle_debug(void);
extern void handle_nmi(void);
extern void handle_breakpoint(void);
extern void handle_overflow(void);
extern void handle_bound_range_exc(void);
extern void handle_inv_opcode(void);
extern void handle_dev_not_available(void);
extern void handle_double_fault(void);
extern void handle_cop(void);
extern void handle_invalid_tss(void);
extern void handle_seg_not_pres(void);
extern void handle_stack_seg_fault(void);
extern void handle_gp_fault(void);
extern void handle_page_fault(void);

/*Exceptions 16-20*/
extern void handle_87_fp_exc(void);
extern void handle_align_check(void);
extern void handle_machine_check(void);
extern void handle_SIMD_fp_exc(void);
extern void handle_virt_exc(void);

/*Exception 30*/
extern void handle_sec_exc(void);
#endif
