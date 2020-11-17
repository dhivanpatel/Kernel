
#include "terminal.h"

static int32_t active_term = 0;
static int32_t sched_term = 0;
static volatile int pit_interrupt_raised = 0;



/*launch_initial_shells
 * this function is called by the pit the first three times to initialize 3 terms
 * INPUTS: which terminal number to init
 * OUTPUTS: None
 * RETURN_VAL: None
 */
void launch_initial_shells(int term_num){
  set_sched_term(term_num);
  terms[term_num].active_process_num = term_num;

  change_vid_mem((char *)terms[term_num].kernel_addr);
  //should I change user-level mapping for this one?

  (terms[term_num]).cursor_x =   get_screen_x(term_num);
  (terms[term_num]).cursor_y =   get_screen_y(term_num);




  PCB_t* next_pcb = (PCB_t *)(EIGHT_MB - (EIGHT_KB *(term_num + 1)));

  next_pcb -> ebp = get_ebp();
  next_pcb ->esp  = get_esp();
  execute((uint8_t *)"shell");
}
//active term indicates what's currently being
//displayed
//sched term indicates the terminal currently scheduled
//did not want to use active term as with alt + fn, active_term
//will not increase in that order
/*terminal_init
 * Initializes all the terminals we need
 * INPUTS: None
 * OUTPUTS: None
 * RETURN_VAL: None
 */
 void terminal_init()
 {
   int i;
   for(i = 0; i < NUM_TERMS; i++)
   {
     if(i == 0)
     {
       //copy all video memory from terminal to this shell;
       //or do we have to?
       //say we boot everything up
       //I guess we also want to copy everything from video to shell 1
       //I guess if the active terminal, we simply input to video screen
       //Gets copied later,
       //but the beginning shell? That must run on first terminal
       terms[i].kernel_addr = (void *)VID_MEM;
       terms[i].shell_on = 1;
       terms[i].active_process_num = -1;
     }
     else if(i == 1)
     {
       terms[i].kernel_addr = (void *) TERM1_MEM;
       terms[i].shell_on = 0;
       terms[i].active_process_num = -1;
     }
     else
     {
       terms[i].kernel_addr = (void *) TERM2_MEM;
       terms[i].shell_on = 0;
       terms[i].active_process_num = -1;
     }
     terms[i].cursor_x = 0;
     terms[i].cursor_y = 0;
     set_terminal_x_y(0,0,i); // set the initial cursor coordinates to zero
     //terms[i].shell_on = 0;
     terms[i].term_num = i;
     uint32_t buf_ind;
     for(buf_ind = 0; buf_ind < SCANCODE_SIZE; buf_ind++)
     {
       terms[i].key_buf[buf_ind] = (char)'\0';
     }
     terms[i].key_buf_idx = 0;
     //now, also do the memset to make sure the
     //three non-displayed memory locations are zero

     char* vidmem;
     switch(i){
       case 0:
        vidmem = (char *)TERM0_MEM;
        //(void) memset((void *)TERM0_MEM, 0, FOUR_KB);
        break;
       case 1:
        vidmem = (char *)TERM1_MEM;
        //(void) memset((void *)TERM1_MEM, 0, FOUR_KB);
        break;
       case 2:
        vidmem = (char *)TERM2_MEM;
        //(void) memset((void *)TERM2_MEM, 0, FOUR_KB);
        break;
       default:
        break;
     }

     int32_t j;
     for (j = 0; j < N_ROWS * N_COLS; j++) {
         *(uint8_t *)(vidmem + (j << 1)) = ' ';
         *(uint8_t *)(vidmem + (j << 1) + 1) = ATTR;
     }
   }
 }


/* terminal_read
 * copies from keyboard buffer into input buffer
  *INPUTS:
  *     fd -- file descriptor
  *     buf -- buffer to copy to w/ keyboard buffer
  *     nbytes -- number of bytes to write
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
  int i, lim;

  while(!enter_pressed){ // spins to wait for enter to be pressed
    continue;
  }

  //gets minimum possible value to copy
  if(nbytes > SCANCODE_SIZE){
    lim = SCANCODE_SIZE;
  } else {
    if(nbytes < key_buf_idx)
      lim = nbytes;
    else
      lim = key_buf_idx;
  }

  for(i = 0; i < SCANCODE_SIZE; i++){
    ((char*)buf)[i] = '\0';//clear buffer
  }
  cli(); // does not allow interrupt during copying
  for(i = 0; i < lim; i++){
    ((char*)buf)[i] = key_buf[i];
    if(((char *)buf)[i] == '\n')
    {
      i++;
      break;
    }
  }
  enter_pressed = 0;
  kb_reset();  // clears keyboard buffer
  sti();
  return i;
}

/* terminal_write
 * copies from keyboard buffer into input buffer
  *INPUTS:
  *     fd -- file descriptor
  *     buf -- buffer to write to terminal
  *     nbytes -- number of bytes to write
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
  int i, lim;

  // if(nbytes > SCANCODE_SIZE){   //gets minimum possible value to copy
  //   lim = SCANCODE_SIZE;
  // } else {
  //   lim = nbytes;
  // }
  lim = nbytes;

  for(i = 0; i < lim; i++){ // outputs char by char from input buffer
    // if(line_idx > MAX_IDX_LINE){
    //   putc(NEW_LINE);
    //   line_idx = 0;
    // }
    putc(((char*)buf)[i]);
    line_idx++;
  }
  line_idx = LINE_START_IDX;

  return i;
}

/* terminal_open
 * copies from keyboard buffer into input buffer
  *INPUTS:
  *       filename -- name of file
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int32_t terminal_open(const uint8_t* filename){
  return 0;
}

/* terminal_close
 * copies from keyboard buffer into input buffer
  *INPUTS:
  *       fd -- file descriptor
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int32_t terminal_close(int32_t fd){
  return 0;
}

/*get active terminal
 * INPUTS: None
 * RETURN VALUE: Number of the terminal being used
 * OUTPUTS: None
 */
int32_t get_active_term()
{
    return active_term;
}

/*change_active_term
* NOTE: Am not sure if we will need this function
 *cyclically increments the active terminal so as to access
 *the next one after an alt+F2
 * INPUTS: new_term: new active terminal
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE_EFFECTS: none
 */
 void change_active_term(int32_t new_term)
 {
   active_term = new_term;
 }

 /*set_sched_term
  *sets the next scheduled terminal
  *the next one after an alt+F2
  * INPUTS: term: new active terminal
  * OUTPUTS: none
  * RETURN VALUE: none
  * SIDE_EFFECTS: none
  */
  void set_sched_term(int term)
  {
    sched_term = term;
    //hope the above works
  }

  /*get_sched_term
   * get the terminal currently being scheduled
   * INPUTS: None
   * RETURN VALUE: the scheduled terminal
   * OUTPUTS: None
   */
  int32_t get_sched_term()
  {
      return sched_term;
  }


 /* switch_terminals
  *Function to switch between terminals
  *INPUTS: next: next terminal to switch to
  *OUTPUTS: None
  *RETURN VALUE: None
  */
  void switch_terminals(int32_t next)
  {
    //uint32_t flags;
    //check for invalid input
    if(next >= NUM_TERMS)
    {
      return;
    }
    //check 'switching' to same place
    if(active_term == next)
    {
      return; //in same terminal, do nothing
    }

    //Otherwise, do the copying
    terminal_t* cur_terminal = &(terms[active_term]);
    terminal_t* next_terminal = &(terms[next]);
    //Then, change the variables accordingly
    cur_terminal -> shell_on = 0;
    next_terminal -> shell_on = 1;
    //switch to corresponding inactive video mem
    switch(active_term)
    {
      case 0:
        cur_terminal -> kernel_addr = (uint8_t *) TERM0_MEM;
        break;
      case 1:
        cur_terminal -> kernel_addr = (uint8_t *) TERM1_MEM;
        break;
      case 2:
        cur_terminal -> kernel_addr = (uint8_t *) TERM2_MEM;
        break;
      default:
        break;
    }

    //not yet,
    //so, first copy everything from video memory to current buffer



    //what else do we copy?
    //right, the video buffer, the keyboard buffer
    //and then the screen x and screen y
    //cli(); //critical section start
    cur_terminal -> cursor_x = get_screen_x(cur_terminal -> term_num);
    cur_terminal -> cursor_y = get_screen_y(cur_terminal -> term_num);
    cur_terminal -> key_buf_idx = key_buf_idx;




    //copy buffer
    int i;
    for(i = 0; i < SCANCODE_SIZE; i++)
    {
      (cur_terminal -> key_buf[i]) = key_buf[i];
    }

    void* copied = memcpy((void *)(cur_terminal -> kernel_addr), (const void *)VID_MEM, N_ROWS * N_COLS * 2);
    //sti(); //critical section end

    //don't do anything more if failure
    if(copied != (void *)(cur_terminal -> kernel_addr))
    {
      return;
    }


    //also change actual video memory if active_term is also sched term
      if(active_term == sched_term)
      {
      //immediately change video memory
      change_vid_mem((char *) cur_terminal -> kernel_addr);
      //Change the vidmap user-address to point to new location
      backlog_vidmap_setup(active_term);
     }
    flush_tlb();

    //new critical section start
    //cli();
    i = 0;
    for(i = 0; i < SCANCODE_SIZE; i++)
    {
      key_buf[i] = next_terminal -> key_buf[i];
    }

    //then also set the new key_buf_idx
    key_buf_idx =  next_terminal -> key_buf_idx;
    //now, copy from new terminal to video
    //cli();
    //make sure that we are setting the values
    //correctly
    int ch = set_screen_x(next_terminal -> cursor_x, next_terminal -> term_num);
    if(ch < 0)
    {
    //s/ti();
      return;
    }

    ch = set_screen_y(next_terminal -> cursor_y, next_terminal -> term_num);
    if(ch < 0)
    {
      //sti();
      return;
    }

    copied = memcpy((void*)VID_MEM, (const void*) next_terminal -> kernel_addr, N_ROWS * N_COLS * 2);
    //sti();
    //new critical section end
    if(copied != (void*)VID_MEM)
    {
      return; //again, failure in copying, so just return;
    }


    //assign this terminal at end
    next_terminal -> kernel_addr = (void *)(VID_MEM);

   if(next == sched_term)
      {
      //immediately change video memory
      change_vid_mem((char *) next_terminal -> kernel_addr);
      //Change the vidmap user-address to point to new location
      vidmap_setup();
    }
    flush_tlb();
    //after everything has been copied
    active_term = next;
    set_cursor();
  }

int get_pit_flag(){
  return pit_interrupt_raised;
}


void set_pit_flag(int val){
  pit_interrupt_raised = val;
}
