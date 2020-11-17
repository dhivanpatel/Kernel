
#ifndef PCB_H
#define PCB_H

#include "types.h"

#define FD_ARR_SIZE 8
#define ARG_SIZE 128
#define PCB_MASK 0xFFFFE000

typedef struct f_ops{
    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open) (const uint8_t* filename);
    int32_t (*close) (int32_t fd);
} f_ops_t;


typedef struct f_desc{
    f_ops_t* file_op_tab_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags; // 0x0000 0000, MSB is present bit, bottom four bits: TERMINAL, RTC, DIR, FILE
} f_desc_t;




typedef struct PCB_struct{

    /*8 files including stdin,stdout */
    f_desc_t file_des_array[FD_ARR_SIZE];
    uint32_t parent_pcb;/*Parent process block pointer to get for return when halting, shell will be NULL*/
    uint8_t argbuf[ARG_SIZE];
    uint32_t arg_length;
    uint32_t process_id;
    uint32_t parent_id;
    uint32_t parent_page_addr; //for paging
    uint32_t page_addr;
    uint32_t parent_ebp;
    uint32_t terminal_num;
    uint32_t vidmap_addr;
    // we save these values in the scheduler
    uint32_t esp;
    uint32_t ebp;
}PCB_t;




#endif
