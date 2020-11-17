#include "syscall.h"
#include "lib.h"
#include "x86_desc.h"
#include "filesys.h"
#include "PCB.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "syshelp.h"
#include "paging.h"

//notes:
//set inode to -1 if file not initialized

#define EIGHT_MB 0x800000
#define FOUR_MB  0x400000
#define EIGHT_KB 8192
#define EIP_ST 24
#define EIP_EN 27
#define BIT_SHIFT 8

#define FILE_PRESENT 0x80
#define NO_FILE      0x00


//#define INT_DIS       4

#define RTC   0x04
#define DIR   0x02
#define FILE  0x01
#define RTC_NUM        0
#define DIRECTORY_NUM  1
#define FILE_NUM       2
#define TERM  0x08

#define FD_START 2
#define FD_END  8

#define USER_START  0x8000000
#define USER_END    0x8400000

#define NUM_TERMS   3
 //I just went with this
                              //If you don't like this, let me know
  /* code */



uint8_t begin_check[BEGIN_BUF_SIZE] = {0x7f, 0x45, 0x4c, 0x46};   // ELF values
static int next_pid = 0;      ///process id

static int current_terminal = 0;
static uint8_t filled_proc[NUM_PROC] = {0,0,0,0,0,0}; //list of filledable processes
//static uint8_t filled_proc[NUM_PROC] = {0,0};


//Empty write syscall function for f ops table, always returns -1
int32_t empty_write(int32_t fd, const void* buf, int32_t nbytes)
{
  return -1;
}
//Empty read syscall function for f ops table, always returns -1
int32_t empty_read (int32_t fd, void* buf, int32_t nbytes)
{
  return -1;
}
//Empty close syscall function for f ops table, always returns -1
int32_t empty_close (int32_t fd)
{
  return -1;
}
//Empty open syscall function for f ops table, always returns -1
int32_t empty_open(const uint8_t* filename)
{
  return -1;
}



//rtc fops table
f_ops_t rtc_fotp = {rtc_read, rtc_write, rtc_open, rtc_close};
//stdin fops table
f_ops_t std_in = {terminal_read, empty_write, empty_open, empty_close};
//stdout fops table
f_ops_t std_out = {empty_read, terminal_write, empty_open, empty_close};
//file fops table
f_ops_t file_fotp = {file_read, file_write, file_open, file_close};
//directory fops table
f_ops_t dir_fotp = {directory_read, directory_write, directory_open, directory_close};



/* get_curr_pcb()
 *  get the current process control block of current process
 * INPUT: none
 * OUTPUT: none
 * RETURN_VAL: pointer to current pcb struct
 * side effects: none
*/
PCB_t* get_curr_pcb(){
  uint32_t esp = get_esp();
  PCB_t* pcb = (PCB_t *) (esp & PCB_MASK);
  return pcb;
}


/* check_fd
 *  check to make sure we dont allocate a file in indexes 0,1 because
 *  that is where stdin and stdout are in the fd array
 * INPUT: fd index
 * OUTPUT: none
 * RETURN_VAL: none
 * side effects: none
*/
int8_t check_fd(int32_t fd){
  if((-1 < fd)  && (fd < FD_END)){ //check for negative and greater than 8
    return 1;
  }
  return 0;
}



/* execute
 *  execute a user level program
 * INPUT: command-> has a filename and arguments
 * OUTPUT: depedning on program something comes to the screen
 * RETURN_VAL: -1 on failure, otherwise returns halt code
 * side effects: starts program
*/
int32_t execute (const uint8_t* command)
{


  int i;
  // okay, so how is the command thing structured?
  //Let's see what we need to do
  uint8_t filename[FILENAME_LEN];
  /*Parse the command and get the arguments*/
  uint32_t char_loc = 0;
  while(char_loc < FILENAME_LEN
    && command[char_loc] != (int8_t)' '
  && command[char_loc] != '\0' && command[char_loc] != '\n')
  {
    filename[char_loc] = command[char_loc];
    char_loc++;
  }

  /*If not reached the last bit, add null to end*/
  if(char_loc < FILENAME_LEN)
  {
      filename[char_loc] = '\0';
  }

  //will this work?
  while (command[char_loc] == ' ')
  {
    char_loc++;
  }

  /*check if valid executable*/

  /*store the arguments somewhere in memory.
   If I'm right, store it right on top of the kernel stack*/
  dentry_t cur_entry;


  // read dentry returns bytes?
  if(read_dentry_by_name((uint8_t *)filename, &cur_entry))
  {
    return -1;
  }

  //check file type
  if(cur_entry.file_type != VALID_FILE)
   {
      return -1;
   }

   //then, do a read_data
   uint8_t file_buf[WHOLE_BUF_SIZE];
   uint32_t cur_node = cur_entry.inode;

   //define a begin_check buffer:

   //read the data into the buffer
   if(read_data(cur_node, 0, file_buf, WHOLE_BUF_SIZE) != WHOLE_BUF_SIZE)
   {
     return -1;
   }
   //then, check for ELF values:
   if(fs_strcmp((uint8_t *) file_buf, (uint8_t *)begin_check, BEGIN_BUF_SIZE))
   {
     return -1;
   }


   for(i = 0; i < NUM_PROC; i++)
   {
     if(!(filled_proc[i]))
     {
       next_pid = i;
       filled_proc[i] = 1;
       break;
     }
   }

   //return -1 if no space to add
   if(i == NUM_PROC)
   {
     return -1;
   }

   //say, it turns out that the next_pid is actually 0,
   //meaning we are executing the first shell...
   //In this case, we do things differently

   //Otherwise, we do the normal route
   //Say we are executing something from the shell.
   //then what happens?
   PCB_t* cur_pcb;
   PCB_t* next_pcb;
   if(next_pid >= NUM_TERMS) //these are for the non-shell programs
   {
      current_terminal = get_active_term();
      /*Not base*/
      cur_pcb  = get_curr_pcb();
      next_pcb = (PCB_t *)(EIGHT_MB - (EIGHT_KB *(next_pid + 1)));
      next_pcb -> process_id = next_pid;
      next_pcb -> parent_id = cur_pcb -> process_id;
      next_pcb -> parent_page_addr = cur_pcb -> page_addr;
      next_pcb -> page_addr = EIGHT_MB + (FOUR_MB * next_pid);
      next_pcb -> parent_pcb = (uint32_t) cur_pcb;
      next_pcb -> vidmap_addr = NULL;
      next_pcb -> terminal_num = cur_pcb -> terminal_num;

      // must also initialize active process number on the terminal

      terms[current_terminal].active_process_num = next_pid;
      /*Initialize file descriptors*/
      for(i = 0; i < FD_ARR_SIZE; i++)
      {
        if(i == 0)
        {
            (next_pcb -> file_des_array[i]).file_op_tab_ptr = &std_in;
            (next_pcb -> file_des_array[i]).flags = (FILE_PRESENT | TERM);

        }
        else if(i == 1)
        {
          (next_pcb -> file_des_array[i]).file_op_tab_ptr = &std_out;
          (next_pcb -> file_des_array[i]).flags = (FILE_PRESENT | TERM);
        }
        else
        {
          (next_pcb -> file_des_array[i]).file_op_tab_ptr = NULL;
          (next_pcb -> file_des_array[i]).flags = NO_FILE;
        }
        (next_pcb -> file_des_array[i]).inode = -1; // garbage val
        (next_pcb -> file_des_array[i]).file_position = 0;

      }
   }
   //for flags:
   // P 0 0 0 T R D F
   else
   {
     current_terminal = get_sched_term();// shell programs
     /*cur_pcb*/
     //since this is first process, parent and child are same
     cur_pcb = (PCB_t *)(EIGHT_MB - (EIGHT_KB *(next_pid + 1)));
     cur_pcb -> process_id = next_pid; // b/c next_pid is zero, or one or two
     cur_pcb -> parent_id = next_pid;
     cur_pcb -> terminal_num = next_pid; //terminal matches itself
     cur_pcb -> parent_page_addr = EIGHT_MB;
     cur_pcb -> page_addr = EIGHT_MB;
     cur_pcb -> parent_pcb = NULL;
     cur_pcb -> vidmap_addr = NULL;

     terms[current_terminal].active_process_num = next_pid; // set active process number for the shells
     next_pcb = cur_pcb; //the price of reusing code

     // here we do not have to change any active process running, because this would be the first exection of shell
     for(i = 0; i < FD_ARR_SIZE; i++)
     {
       if(i == 0)
       {
           (next_pcb -> file_des_array[i]).file_op_tab_ptr = &std_in;
           (next_pcb -> file_des_array[i]).flags = (FILE_PRESENT | TERM);

       }
       else if(i == 1)
       {
         (next_pcb -> file_des_array[i]).file_op_tab_ptr = &std_out;
         (next_pcb -> file_des_array[i]).flags = (FILE_PRESENT | TERM);
       }
       else
       {
         (next_pcb -> file_des_array[i]).file_op_tab_ptr = NULL;
         (next_pcb -> file_des_array[i]).flags = NO_FILE;
       }
       (next_pcb -> file_des_array[i]).inode = -1; // garbage val
       (next_pcb -> file_des_array[i]).file_position = 0;
     }
     //set everything from start, as no prev process
   }

   //initialize argbuf to all zeros
   for(i = 0; i < ARG_SIZE; i++)
   {
     (next_pcb -> argbuf[i]) = '\0';
   }

  uint32_t argl = 0;
  //if n args were passed through, there should be a zero,
  //so we don't bother

  if(command[char_loc] != '\0')
  {
    //make process control block store the args for getargs()
    for(i = 0; i < SCANCODE_SIZE; i++)
    {
      if(command[char_loc] != '\n' && char_loc < SCANCODE_SIZE && command[char_loc] != '\0')
      {
        (next_pcb -> argbuf[i]) = command[char_loc++];
        argl++;
      }
      else
      {
        break; //if reached end of buffer, we simply get out of loop
      }
    }
  }

  if(i < SCANCODE_SIZE)
  {
    (next_pcb -> argbuf[i]) = '\0'; //add null character at end
  }

  next_pcb -> arg_length = argl;
   //set up the current PID
   // what else do we do???




   //add paging to the structure, before loading the exec
   /////////////////////////////// PAGINGOUYEYGUYFGU#Gu

   uint32_t phys_addr;

   phys_addr = next_pcb -> page_addr;

   if(program_paging_setup(VIRTUAL_PROCESS_START, phys_addr)){
     return -1;
   }

   flush_tlb();

   //do the task switching
   tss.esp0 = EIGHT_MB - (EIGHT_KB * next_pid) - INT_DIS; // -4 is so that starts at x7FFFFF
   sti();// as new program at 8MB





   uint32_t new_address = 0;
   //construct new address from filebuf
   //this is the address from which EIP starts executing
   //the new program  =
   uint32_t* new_addr_ptr = (uint32_t *)(&file_buf[EIP_ST]);
   new_address = *new_addr_ptr;

   load_exec_to_page(cur_node); //cur node should have inode of file

   int32_t retval = start_exec(new_address, (uint32_t)(&(next_pcb -> parent_ebp)));

   //add more stuff onto PCB
   //finish execute
   //If we are at shell:

   /*********We'll worry about this later****************
   if(!next_pid)
   {
     //call execute again, but clear the available bit
     filled_proc[0] = 0;
     execute((uint8_t *)"shell");
   }
   */
   return retval;
}


/* halt
 *
 * halt the process and return to the parent process
 * INPUT: status
 * OUTPUT: status, help return to parent process
 * RETURN_VAL: -1 if something messes up, otherwise returns control to parent
 * side effects: terminates the current process
*/
int32_t halt(uint8_t status){
  cli();
  PCB_t* curr_pcb = get_curr_pcb();
  PCB_t* parent_pcb = (PCB_t*) (curr_pcb->parent_pcb);

  filled_proc[curr_pcb->process_id] = 0; //say that the current process is done


  //clear the file descriptor
  int i;
  for(i = 0; i < FD_ARR_SIZE; i++){

    // close any open/ active files
    if(((curr_pcb -> file_des_array[i]).flags & FILE_PRESENT) == FILE_PRESENT){
      close(i);
    }
    (curr_pcb -> file_des_array[i]).file_op_tab_ptr = NULL;
    (curr_pcb -> file_des_array[i]).flags = NO_FILE;

    /*not sure what to modify these to and if they need modification*/
    (curr_pcb -> file_des_array[i]).inode = -1;
    (curr_pcb -> file_des_array[i]).file_position = 0;
  }
  // RESTART SHELL IF LAST THING TO HALT
  if(curr_pcb->process_id == curr_pcb->parent_id) {
    terms[curr_pcb -> terminal_num].active_process_num = curr_pcb->process_id;
    execute((uint8_t *)"shell");
  }


  //check if it's the first shell
  if(parent_pcb != NULL){


    //curr_pcb -> vidmap_addr = 0;
    //handle the parent paging
    if(program_paging_setup(VIRTUAL_PROCESS_START, parent_pcb -> page_addr)){ //dhivan wrote this helper function, this could be a potential spot for error
      return -1;
    }
    //handle the task switching
    tss.esp0 = EIGHT_MB - (EIGHT_KB * (parent_pcb -> process_id)) - INT_DIS; // -4 is so that starts at x7FFFFF

    flush_tlb();

    terms[curr_pcb -> terminal_num].active_process_num = curr_pcb -> parent_id;      //// set the active process number to the parent ID here
  }
  sti();

  //jump to execute return
  ret_to_exec(curr_pcb -> parent_ebp, status);


  return -1; //if you make it here something went wrong so return -1

}





/* open
 *
 * open certain file type
 * INPUT: filename
 * OUTPUT: none
 * RETURN_VAL: -1 if something messes up, returns 0 for succseful open
 * side effects: after opening we can do read/write etc
*/
int32_t open(const uint8_t* filename){
  PCB_t* curr_pcb = get_curr_pcb();
  int32_t fd;
  dentry_t file_dentry;

  if(read_dentry_by_name(filename, &file_dentry)){
    return -1;
  }

  for(fd = FD_START; fd < FD_END; fd++){
    if(curr_pcb->file_des_array[fd].flags == NO_FILE){
      break;
    }
  }

  if(fd == FD_END){
    return -1;
  }

  uint32_t file_type = file_dentry.file_type;
  switch(file_type){
    case RTC_NUM:
      if(rtc_open(filename)){
        return -1;
      }
      curr_pcb->file_des_array[fd].file_op_tab_ptr = &rtc_fotp;
      curr_pcb->file_des_array[fd].inode = 0;
      curr_pcb->file_des_array[fd].flags = FILE_PRESENT | RTC;
      return fd;
    case DIRECTORY_NUM:
      if(directory_open(filename)){
        return -1;
      }
      curr_pcb->file_des_array[fd].file_op_tab_ptr = &dir_fotp;
      curr_pcb->file_des_array[fd].inode = 0;
      curr_pcb->file_des_array[fd].flags = FILE_PRESENT | DIR;
      return fd;
    case FILE_NUM:
      if(file_open(filename)){
        return -1;
      }
      curr_pcb->file_des_array[fd].file_op_tab_ptr = &file_fotp;
      curr_pcb->file_des_array[fd].inode = file_dentry.inode;
      curr_pcb->file_des_array[fd].flags = FILE_PRESENT | FILE;
      return fd;
  }

  return fd;
}

/* close
 *
 * close certain filetype
 * INPUT: fd-> index into fd array in pcb
 * OUTPUT: none
 * RETURN_VAL: -1 if something messes up, return 0 which means close happened succesfully
 * side effects: closes file succesfully
*/
int32_t close (int32_t fd){
  if(!check_fd(fd)){
    return -1;
  }

  PCB_t* curr_pcb  = get_curr_pcb();

  if(curr_pcb->file_des_array[fd].flags == NO_FILE){
    return -1;
  }

  if((curr_pcb->file_des_array[fd].file_op_tab_ptr) -> close(fd)){
    return -1;
  }

  curr_pcb->file_des_array[fd].file_op_tab_ptr = NULL;
  curr_pcb->file_des_array[fd].flags = NO_FILE;
  curr_pcb->file_des_array[fd].inode = -1;    // garbage val
  curr_pcb->file_des_array[fd].file_position = 0;
  return 0;
}

/*
 * getargs
 * copies the argument into the user level buffer
 * INPUTS: screen_start = pointer to address of video_memory
 * RETURN VALUE: 0 if success; -1 if fail
 * SIDE EFFECTS: modifies video buffer of terminal
 */
int32_t getargs (uint8_t* buf, int32_t nbytes)
{
  PCB_t* curr_pcb = get_curr_pcb();
    if((nbytes < (curr_pcb->arg_length)) || (curr_pcb -> arg_length == 0)){ //check if the arguments are too long
      return -1;
    }
    strncpy((int8_t*)buf, (int8_t*)curr_pcb->argbuf, nbytes); //copy the contents the buffer

    return 0; //success if we reach this point

}


/*
 * vidmap
 * maps video memory, located in the memory location passed
 * onto the page table
 * and also the terminal
 * INPUTS: screen_start = pointer to address of video_memory
 * RETURN VALUE: 0 if success; -1 if fail
 * SIDE EFFECTS: modifies video buffer of terminal
 */
int32_t vidmap (uint8_t** screen_start)
{
   //the address that vidmap always sets to:
   //0x08CB8000
   //basically, screen_start is the user_space address of the
   //variable that stores the video memory address
   //so, we set up paging for this new address (decided to be 0x08CB8000)
   //after which we just dereference screen_start and
   //assign this new address to our pointer
  // so far, here's what happening:
    //a frame gets printed, but only the I and M toggle
    //pretty much everything else stays the same
    //possibly a problem with RTC interrupt, but
    //I'm not sure

  //make sure the pointer to the video memory address is in the
  //user space
  if(!(((uint32_t) screen_start >= USER_START) && ((uint32_t) screen_start < USER_END)))
  {
    return -1;
  }



  PCB_t* pcb = get_curr_pcb();
  pcb -> vidmap_addr = USER_VID_ST;

  uint32_t vidmapAddr = (uint32_t) terms[pcb -> terminal_num].kernel_addr;
  //here, get the correct terminal and get that address' vidmap

  vidmap_modify(vidmapAddr);
  flush_tlb();
  //change the location of the video memory

  (*screen_start) = (uint8_t *) USER_VID_ST;

  return 0;
}
//not implemented yet
int32_t set_handler (int32_t signum, void* handler_address)
{
  return -1;
}
//not implemented yet
int32_t sig_return (void)
{
  return -1;
}

/* write
 *
 * writes a buffer to something-->terminal etc
 * INPUT: fd-> fd array index, buffer we want to write, nbytes-> number of bytes to write
 * OUTPUT: terminal write usually
 * RETURN_VAL: -1 if invalid, 0 on success
 * side effects: writes a buffer, to screen
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes){

  if(!check_fd(fd)){
    return -1;
  }


  PCB_t* curr_pcb  = get_curr_pcb();
  if(curr_pcb->file_des_array[fd].flags == NO_FILE){
    return -1;
  }

  if(!buf){
    return -1;
  }

  if(nbytes < 0){
    return -1;
  }
  return (curr_pcb->file_des_array[fd].file_op_tab_ptr) -> write(fd, buf, nbytes);

}

/* read
 *
 * halt the process and return to the parent process
 * INPUT: fd-> index to fd array, buf-> buffer that is built when read is called, nbytes-> num bytes to read
 * OUTPUT: none
 * RETURN_VAL: -1 if something messes up, 0 on succesful read
 * side effects: can read from terminal usua;;y
*/
int32_t read (int32_t fd, void* buf, int32_t nbytes){

  if(!check_fd(fd)){
    return -1;
  }

  sti();
  PCB_t* curr_pcb  = get_curr_pcb();

  if(curr_pcb->file_des_array[fd].flags == NO_FILE){
    return -1;
  }

  if(!buf){
    return -1;
  }

  // if(!check_fd(fd)){
  //   return -1;
  // }

  if(nbytes < 0){
    return -1;
  }

  return (curr_pcb->file_des_array[fd].file_op_tab_ptr) -> read(fd, buf, nbytes);

}
