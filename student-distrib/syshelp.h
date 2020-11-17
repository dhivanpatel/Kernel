#ifndef SYS_HELP_H
#define SYS_HELP_H

/*helper function to start executing user level program*/
uint32_t start_exec(uint32_t new_address, uint32_t pcb_ebp);

uint32_t get_esp();

uint32_t get_ebp();

/*After halt is called in user level program, we return to parent process*/
uint32_t ret_to_exec(uint32_t parent_ebp, uint32_t halt_val);

void run_next_proc(uint32_t current_ebp, uint32_t current_esp);
#endif
