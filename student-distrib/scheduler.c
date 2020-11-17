

#include "scheduler.h"


#define TWENTY 11932
int terms_init = 0;     // this is to statically execute three diff shells for three terms its a counter


 /*pit_init
  *Initialize any PIC ports we need and the Programmable_Interval_Timer
  *INPUTS: the frequency/ quantum time we want
  *OUTPUTS: none
  *RETURN VALUE: none
  */
void pit_init(){

  // int divisor = INPUT_CLOCK / hz;
  outb(SQUARE_WAVE, COMMAND_REG);             /* Set our command byte 0x36 */
  outb(TWENTY & 0xFF, CHANNEL_ZERO);   /* Set low byte of divisor */       // mask out part of it
  outb(TWENTY >> 8, CHANNEL_ZERO);     /* Set high byte of divisor */  //  by shifting it right 8


  enable_irq(PIT_IRQ);  // irq 0 is the system timer


}


/*schedule
 *PIT interrupt handler used to schedule round robin, first three interrupts initialize shells
 *
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: none
 */

void schedule(){

  uint32_t current_term;
  uint32_t next_term;
  cli();
  send_eoi(PIT_IRQ);
  current_term = get_sched_term();

  next_term = (current_term + 1) % NUM_TERMS;

  if(terms_init < NUM_TERMS){// have three terminals been initialized yet
    launch_initial_shells(terms_init++);
  }
  /// launch first three shells

        // force process to go back so we finish enter to execute
        if(enter_pressed){
          next_term = get_active_term();
          set_pit_flag(1);  // set this back to 1
        }

         (terms[current_term]).cursor_x =   get_screen_x(get_sched_term());
         (terms[current_term]).cursor_y =   get_screen_y(get_sched_term());
        //first, store current ebp for leaving process
        PCB_t* current_pcb = (PCB_t *)(EIGHT_MB - (EIGHT_KB *(terms[current_term].active_process_num + 1)));

        current_pcb -> ebp = get_ebp(); //this way, we store the current ebp into our thing
        current_pcb -> esp = get_esp();


        change_vid_mem((char *)terms[next_term].kernel_addr);


        //here we schedule the next thing
        int32_t active_process = terms[next_term].active_process_num;

        //get next PCB

          PCB_t* next_pcb;


          next_pcb = (PCB_t *)(EIGHT_MB - (EIGHT_KB *(active_process + 1)));

        // uint32_t hello = (uint32_t)next_pcb -> page_addr;
        // set up program paging for next process
        if(program_paging_setup(VIRTUAL_PROCESS_START,next_pcb -> page_addr)){
          sti();
          return;
        }
        //we also need to change the user-level address
        vidmap_modify((uint32_t) terms[next_term].kernel_addr);
        flush_tlb();
        //now, we modify TSS.esp0 to point to this place, which is the bottom of the next process stack
        tss.esp0 = EIGHT_MB - (EIGHT_KB * active_process) - INT_DIS;
        tss.ss0 = KERNEL_DS;
        set_sched_term(next_term);
        //here we restore esp and ebp, that we saved at the begining


        asm volatile(
          "movl %0, %%ebp \n"
          :
          :"a"(next_pcb->ebp)
        );
        // asm volatile(
        //               "ret\n");

        // run_next_proc(next_pcb -> ebp, next_pcb -> esp); // maybe we need esp too
        sti();
}
