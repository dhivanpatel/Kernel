/*Paging header file set up*/
#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

/* Defining some constants here to initialize
 * page directory, table, then page
 */

#define ONE_KILOBYTE         1024
#define FOUR_KILOBYTES       ONE_KILOBYTE * 4

#define VIRTUAL_PROCESS_START 0x8000000
#define USER_VID_ST           0x8CB8000

/*Page directory and page table are the same size*/
uint32_t page_directory[ONE_KILOBYTE] __attribute__((aligned(FOUR_KILOBYTES)));

uint32_t page_table_one[ONE_KILOBYTE] __attribute__((aligned(FOUR_KILOBYTES)));

uint32_t page_table_vidmap[ONE_KILOBYTE] __attribute__((aligned(FOUR_KILOBYTES)));




/*Initialize our paging indirection by mapping a page table entry to video memory
  and mapping a page directory entry to kernel memory*/
 int32_t paging_init();

 void flush_tlb();

 int32_t program_paging_setup(uint32_t virtualPageAddress, uint32_t  physicalMemoryAddress);

int32_t vidmap_clear(uint32_t vidmapAddress);
int32_t vidmap_setup();

int32_t vidmap_modify(uint32_t videoAddr);

int32_t backlog_vidmap_setup(int term_num);
// /*Enables paging in control register 0*/
// extern void enable_paging();
//
// /*Loads page_directory starting address into control register 3*/
// extern void load_directory();
//
// /*Sets up the page size extension bit*/
// extern void extension();



/*REFERENCES
https://wiki.osdev.org/Paging

https://wiki.osdev.org/CPU_Registers_x86

https://wiki.osdev.org/Setting_Up_Paging
 */




#endif  /* _PAGING_H */
