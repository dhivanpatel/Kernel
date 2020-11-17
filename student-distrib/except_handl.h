#ifndef EXCEPT_HANDL_H
#define EXCEPT_HANDL_H

// #include "exceptions.h"

/*Exceptions 0 through 14*/
extern void exc_div_by_zero(void);
extern void exc_debug(void);
extern void exc_nmi(void);
extern void exc_break(void);
extern void exc_overflow(void);
extern void exc_bound_range(void);
extern void exc_invalid_op(void);
extern void exc_dev_not_available(void);
extern void exc_double_fault(void);
extern void exc_coprocessor(void);
extern void exc_invalid_tss(void);
extern void exc_seg_not_pres(void);
extern void exc_stack_seg_fault(void);
extern void exc_gp_fault(void);
extern void exc_page_fault(void);

/*Exceptions 16-20*/
extern void exc_87_fp_exc(void);
extern void exc_align_check(void);
extern void exc_machine_check(void);
extern void exc_simd_fp_exc(void);
extern void exc_virt_exc(void);

/*Exception 30*/
extern void exc_sec_exc(void);


#endif
