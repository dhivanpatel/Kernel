#ifndef RTC_H
#define RTC_H

#define RTC_INTERRUPT 8
#define RTC_VEC 0x28
#define RTC_CMD_PORT 0x70
#define RTC_DATA_PORT 0x71

#define RTC_A_DISABLENMI 0x8A
#define RTC_B_DISABLENMI 0x8B
#define RTC_B_ENABLENMI 0x0B
#define TURN_ON_FLAG 0x40
#define REGISTER_C 0x0C

#define DIVIDER_MASK 0xF0


//got from page 19 of https://courses.engr.illinois.edu/ece391/sp2019/secure/references/ds12887.pdf
#define RATE2 0xF
#define RATE4 0xE
#define RATE8 0xD
#define RATE16 0xC
#define RATE32 0xB
#define RATE64 0xA
#define RATE128 0x9
#define RATE256 0x8
#define RATE512 0x7
#define RATE1024 0x6

//the frequency that we want to set the rtc to
#define HERTZ2 2
#define HERTZ4 4
#define HERTZ8 8
#define HERTZ16 16
#define HERTZ32 32
#define HERTZ64 64
#define HERTZ128 128
#define HERTZ256 256
#define HERTZ512 512
#define HERTZ1024 1024


#include "types.h"

extern void test_handle_rtc(void);
extern void handle_rtc(void);
extern void rtc_setup();
extern int rtc_open(const uint8_t* filename);
extern int rtc_close(int32_t fd);
extern int rtc_read(int32_t fd, void* buf, int32_t nbytes);
extern int rtc_write(int32_t fd, const void* buf, int32_t nbytes);



#endif
