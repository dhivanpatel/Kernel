#include "rtc.h"
#include "i8259.h"
#include "lib.h"

#define NUM_COLS    80
#define NUM_ROWS    25


int intFlag;
static char* video_mem = (char*) 0xB8000;
/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/*test_handle_rtc
 *test handler for rtc. handles test_interrupts
 *INPUT: none
 *OUTPUT: none
 * side effects: modifies screen
 */
void test_handle_rtc(void)
{
    send_eoi(RTC_INTERRUPT);
    intFlag = 0; //set the intFlag to 0 to allow the read to finish
    //send_eoi(2);
    outb(REGISTER_C, RTC_CMD_PORT);   // here we clear register c
    inb(RTC_DATA_PORT);
    //test_interrupts();
}

/*dummy*/
void handle_rtc(void)
{
    send_eoi(RTC_INTERRUPT);
}


/* rtc_setup
 *
 * setup RTC to start interacting with PIC and PC
 * INPUT: none
 * OUTPUT: none
 * RETURN_VAL: none
 * side effects: inits rtc and sets the rate to 2 Hz
*/
void rtc_setup()
{
      uint8_t p;
      int rate;
      outb(RTC_B_DISABLENMI, RTC_CMD_PORT); //select register B and also disable nmi
      p = inb(RTC_DATA_PORT); //get initial value
      outb(RTC_B_DISABLENMI, RTC_CMD_PORT); //send data again to reset to B
      outb(p | TURN_ON_FLAG, RTC_DATA_PORT);


      //now, enable PIC interrupts
      enable_irq(RTC_INTERRUPT);
      rate = RATE2; //rate for 2 Hz
      outb(RTC_A_DISABLENMI, RTC_CMD_PORT); //select register A and disable nmi
      p = inb(RTC_DATA_PORT); //get initial value
      outb(RTC_A_DISABLENMI, RTC_CMD_PORT); //send data again to reset A
      outb((p & DIVIDER_MASK) | rate, RTC_DATA_PORT); //write the rate to the bottom 4 bits of register A
      enable_irq(RTC_INTERRUPT);

}

/* rtc_open
 *
 * Set the RTC frequency to 2
 * INPUT: pointer to the filename (not used)
 * OUTPUT: none
 * RETURN_VAL: 0 for success
 * side effects: sets the RTC frequency to 2
*/
int rtc_open(const uint8_t* filename){
  uint8_t p;
  int rate;
  rate = RATE2; //get the rate for 2 Hz
  outb(RTC_A_DISABLENMI, RTC_CMD_PORT); //select register A and disable nmi
  p = inb(RTC_DATA_PORT); //get initial value
  outb(RTC_A_DISABLENMI, RTC_CMD_PORT);
  outb((p & DIVIDER_MASK) | rate, RTC_DATA_PORT); //write the rate to the bottom 4 bits of register A
  enable_irq(RTC_INTERRUPT);
  return 0;
}


/* rtc_close
 *
 * does nothing at the moment
 * INPUT: file descriptor
 * OUTPUT: none
 * RETURN_VAL: 0 for success
 * side effects: none
*/
int rtc_close(int32_t fd){
  return 0;
}


/* rtc_read
 *
 * waits for an interrupt from the RTC
 * INPUT: file descriptor, buffer, number of bytes passed in
 * OUTPUT: none
 * RETURN_VAL: 0 for success
 * side effects: none
*/
int rtc_read(int32_t fd, void* buf, int32_t nbytes){
  while(intFlag){ //infinite loop until an interrupt has been called
  }
  intFlag = 1; //reset the interrupt flag
  return 0;
}


/* rtc_write
 *
 * changes the RTC frequency
 * INPUT: file descriptor
          frequency passed in via buffer
          number of bytes wanting to write
 * OUTPUT: none
 * RETURN_VAL: 0 for success, -1 for failure
 * side effects: changes the rtc frequency
*/
int rtc_write(int32_t fd, const void* buf, int32_t nbytes){
  int32_t rate;
  uint8_t p;
  int32_t freq;
  if(buf == NULL || nbytes != 4){ //check for invalid input and that you are writing 4 bytes
    return -1;
  }
  freq = *((int32_t*)buf); //get the freq that we wish to set
  if(freq == HERTZ2) rate = RATE2; //set the rate for the desired frequency
  else if(freq == HERTZ4) rate = RATE4;
  else if(freq == HERTZ8) rate = RATE8;
  else if(freq == HERTZ16) rate = RATE16;
  else if(freq == HERTZ32) rate = RATE32;
  else if(freq == HERTZ64) rate = RATE64;
  else if(freq == HERTZ128) rate = RATE128;
  else if(freq == HERTZ256) rate = RATE256;
  else if(freq == HERTZ512) rate = RATE512;
  else if(freq == HERTZ1024) rate = RATE1024;
  else{
    return -1;
  }
  outb(RTC_A_DISABLENMI, RTC_CMD_PORT); //select register A and disable nmi
  p = inb(RTC_DATA_PORT); //get initial value
  outb(RTC_A_DISABLENMI, RTC_CMD_PORT); //send data again to reset A
  outb((p & DIVIDER_MASK) | rate, RTC_DATA_PORT); //write the rate to lower 4 bits of register A
  enable_irq(RTC_INTERRUPT);
  return nbytes;
}
