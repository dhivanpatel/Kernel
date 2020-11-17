#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "i8259.h"
#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"
#include "PCB.h"
#include "syshelp.h"
#include "scheduler.h"
#include "keyboard.h"

#define NUM_TERMS 3
#define VID_MEM 0xB8000
#define TERM0_MEM 0xB9000
#define TERM1_MEM 0xBA000
#define TERM2_MEM 0xBB000
#define FOUR_KB   4096
#define N_ROWS 80
#define N_COLS 25
#define ATTR 0x7


//create a terminal struct that holds the
//user level buffer pointer, the kernel level pointer, the value and everythin
typedef struct terminal_struct{
  //the video memory kernel addr will only change

  //now, have cursor, (start x and start y)
  void* kernel_addr; //kernel addr (the actual page)
  uint32_t cursor_x;
  uint32_t cursor_y;

  uint8_t shell_on;
  uint32_t term_num;      // 0, 1, 2
  int32_t active_process_num; // 0, 1, 2, 3, 4, 5 etc however many processes
  //will use -1 to indicate no process is being run

  char key_buf[SCANCODE_SIZE];
  int key_buf_idx;
} terminal_t;


terminal_t terms[NUM_TERMS];



//uint32_t active_term = 0;
//terminal_t[3] cur_terminals;
extern void launch_initial_shells(int term_num);
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd,const void* buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);
extern int32_t get_active_term();
extern int32_t get_sched_term();
extern void set_sched_term(int term);
extern void switch_terminals(int32_t next);
extern void terminal_init();
extern int get_pit_flag();
extern void set_pit_flag(int val);
#endif
