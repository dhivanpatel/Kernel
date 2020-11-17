/*Function(s) to handle syscalls*/
#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_VEC 0x80

#define BEGIN_BUF_SIZE 4
#define WHOLE_BUF_SIZE 40
#define NUM_PROC 6 // once scheduling is implmented
#define INT_DIS  4


#include "types.h"
#include "PCB.h"
//  void syscall();
    int32_t halt (uint8_t status);
    int32_t execute (const uint8_t* command);
    int32_t read(int32_t fd, void *buf, int32_t nbytes);
    int32_t write(int32_t fd, const void* buf, int32_t nbytes);
    int32_t open (const uint8_t* filename);
    int32_t close (int32_t fd);
    int32_t getargs (uint8_t* buf, int32_t nbytes);
    int32_t vidmap (uint8_t** screen_start);
    int32_t set_handler (int32_t signum, void* handler_address);
    int32_t sigreturn (void);

    PCB_t* get_curr_pcb();

    int32_t empty_read (int32_t fd, void* buf, int32_t nbytes);
    int32_t empty_write(int32_t fd, const void* buf, int32_t nbytes);
    int32_t empty_open(const uint8_t* filename);
    int32_t empty_close (int32_t fd);

    extern void handle_syscalls(void);





#endif
