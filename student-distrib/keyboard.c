#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
/*Won't need these, yet ...*/
//int caps_lock_on = 0;
//int shift_on = 0;
/*This file handles the keyboard functions
 *and eventually will become the keyboard driver*/

/*List of scancodes for a keyboard - with no shift or anything */
/*Credit goes to Bran's Kernel development tutorialt
  Comments inside array from his tutorial*/
 unsigned char kbdus[SCANCODE_SIZE] =
 {
     0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
   '9', '0', '-', '=', '\b',	/* Backspace */
   '\t',			/* Tab */
   'q', 'w', 'e', 'r',	/* 19 */
   't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
     0,			/* 29   - Control */
   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
  '\'', '`',   0,		/* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
   'm', ',', '.', '/',   0,				/* Right shift */
   '*',
     0,	/* Alt */
   ' ',	/* Space bar */
     0,	/* Caps lock */
     0,	/* 59 - F1 key ... > */
     0,   0,   0,   0,   0,   0,   0,   0,
     0,	/* < ... F10 */
     0,	/* 69 - Num lock*/
     0,	/* Scroll Lock */
     0,	/* Home key */
     0,	/* Up Arrow */
     0,	/* Page Up */
   '-',
     0,	/* Left Arrow */
     0,
     0,	/* Right Arrow */
   '+',
     0,	/* 79 - End key*/
     0,	/* Down Arrow */
     0,	/* Page Down */
     0,	/* Insert Key */
     0,	/* Delete Key */
     0,   0,   0,
     0,	/* F11 Key */
     0,	/* F12 Key */
     0,	/* All other keys are undefined */
 };

/*scancodes for keys with shift on*/
 unsigned char kbdus_shift[128] =
 {
     0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
   '(', ')', '_', '+', '\b',	/* Backspace */
   '\t',			/* Tab */
   'Q', 'W', 'E', 'R',	/* 19 */
   'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,	/* Enter key */
     0,			/* 29   - Control */
   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
  '"', '~',   0,		/* Left shift */
  '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
   'M', '<', '>', '?',   0,				/* Right shift */
   '*',
     0,	/* Alt */
   ' ',	/* Space bar */
     0,	/* Caps lock */
     0,	/* 59 - F1 key ... > */
     0,   0,   0,   0,   0,   0,   0,   0,
     0,	/* < ... F10 */
     0,	/* 69 - Num lock*/
     0,	/* Scroll Lock */
     0,	/* Home key */
     0,	/* Up Arrow */
     0,	/* Page Up */
   '-',
     0,	/* Left Arrow */
     0,
     0,	/* Right Arrow */
   '+',
     0,	/* 79 - End key*/
     0,	/* Down Arrow */
     0,	/* Page Down */
     0,	/* Insert Key */
     0,	/* Delete Key */
     0,   0,   0,
     0,	/* F11 Key */
     0,	/* F12 Key */
     0,	/* All other keys are undefined */
 };

/* keyboard_init
 * initializes keyboard by enabling the IRQ pin
 * on PIC
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void keyboard_init()
{
  enable_irq(KEYBOARD_INTERRUPT);
    enter_pressed = 0;
    key_buf_idx = 0;
    line_idx = LINE_START_IDX;
    keyboard_status = 0x0000;
}

/* kb_reset
 * resets keyboard buffer and other vars
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void kb_reset(){
  int i;
  for(i = 0; i < SCANCODE_SIZE; i++){
    key_buf[i] = 0;
  }
  key_buf_idx = 0;
  line_idx = LINE_START_IDX;
}

int check_scancode(int scancode){

  int spec = 0;
  switch (scancode) { // CHECKS for all special characters
    case SPEC1:
      spec = 1;
      break;
    case SPEC2:
      spec = 1;
      break;
    case SPEC3:
      spec = 1;
      break;
    case SPEC4:
      spec = 1;
      break;
    case SPEC5:
      spec = 1;
      break;
    case SPEC6:
      spec = 1;
      break;
    case SPEC7:
      spec = 1;
      break;
    case SPEC8:
      spec = 1;
      break;
    case SPEC9:
      spec = 1;
      break;
  }
  if(scancode < SPECIAL_CHARS || spec)  //checks initial spec chars
    return 1;
  return 0;
}


/* handle_char
 * handles character with scancode, adds it to key_buf
 * INPUT: scancode
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_char(int scancode){
  char c;
  //check for the alts+f first
  if(keyboard_status & ALT_MASK)
  {
    if(scancode >= F1 && scancode <= F3)
    {
      //this should give the right terminal
      switch_terminals(scancode - F1);
      // set_cursor();
      return;
    }
  }
  if(kbdus[scancode] == 0){
    return;
  } else if(kbdus_shift[scancode] == 0){
    return;
  }
  if(keyboard_status & CTRL_MASK){
    switch(scancode){
      case CLEAR_SCREEN:
        clear_terminal();
        set_coord(LINE_START_IDX, ZERO);
        set_cursor();
        return;
    }
  }


  int shift_on = keyboard_status & SHIFT_MASK;
  int caps_on = keyboard_status & CAPS_MASK;

  if(shift_on && !(caps_on)){ // just shift
    c = kbdus_shift[scancode];
  } else if(caps_on && !(shift_on)){ // just caps
    if(check_scancode(scancode)){
      c = kbdus[scancode];
    } else{
      c = kbdus_shift[scancode];
    }
  } else if(caps_on && shift_on){ // just caps
    if(check_scancode(scancode)){
      c = kbdus_shift[scancode];
    }
    else{
      c = kbdus[scancode];
    }
  } else{ // normal
    c = kbdus[scancode];
  }

  key_buf[key_buf_idx++] = c; // adds char to keyboard buffer
  line_idx++;
  vid_putc(c);
  
}


/* handle_enter
 * handles the enter key, prints the newline to screen
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_enter(){

  enter_pressed = ON; // enter flag
  key_buf[key_buf_idx++] = NEW_LINE_KB;
  sti();
  while(!get_pit_flag()){
  }
 set_pit_flag(0);  // reset it to 0
  putc(NEW_LINE_KB);
}


/* handle_shift
 * handles shift key for upper case
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_shift(){
  keyboard_status = keyboard_status | SHIFT_MASK; // sets shift bit
}

/* handle_backspace
 * handles backspace, and decreases the buffer/erases from the buffer
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_backspace(){
  if(!line_idx && key_buf_idx){ // if mid key buffer and at star of line
      key_buf[--key_buf_idx] = EMPTY_CHAR;
      line_idx = MAX_IDX_LINE;
      clear_char_nl();
  } else if(line_idx && key_buf_idx){ // regular case
    key_buf[--key_buf_idx] = EMPTY_CHAR;
    line_idx--;
    clear_char();
  }


}


/* handle_ctrl
 * sets the ctrl key bit
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_ctrl(){
  keyboard_status = keyboard_status | CTRL_MASK; //sets ctrl bit
}


/*handle_alt
* sets the alt key bit
* INPUT: None
* OUTPUT: None
* RETURN VALUE: None
*/
void handle_alt(){
  keyboard_status = keyboard_status | ALT_MASK; //sets alt bit
}



/* handle_caps_lock
 * handles the cap_lock by setting
 * INPUT: None
 * OUTPUT: None
 * RETURN VALUE: None
 */
void handle_caps_lock(){
  if(keyboard_status & CAPS_MASK){
    keyboard_status = keyboard_status & DISABLE_CAPS; // disables caps bit
  } else{
    keyboard_status = keyboard_status | CAPS_MASK; // sets caps bit
  }
}


/*test_handle_keyboard
 *test function to handle keyboard. just echoes alphanumeric
 *characters on screen
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: none
 */
void test_handle_keyboard(void)
{
  cli(); // critical section

  send_eoi(KEYBOARD_INTERRUPT);
  uint8_t scancode = inb(KEYBOARD_PORT);


  if(enter_pressed){ // handles reset if enter was pressed previously
    kb_reset();
  }

  switch (scancode) { //handles enter and backspace
    case ENTER:
      handle_enter();
      sti();
      return;
    case BACKSPACE:
      handle_backspace();
      set_cursor();
      sti();
      return;
  }
  //now, we need to do putc
  //doesn't look right, but we'll see
  if(key_buf_idx > 127){ //check if id is greater than scancode size
    sti();
    return;
  } else if(line_idx > MAX_IDX_LINE){ // end of line
    putc(NEW_LINE_KB);
    line_idx = 0;
  }

  if(!(scancode & KEYBOARD_RELEASE)) // handles all keyboard scancodes
  {
    switch(scancode){
      case LEFT_SHIFT:
        handle_shift();
        break;
      case RIGHT_SHIFT:
        handle_shift();
        break;
      case CTRL:
        handle_ctrl();
        break;
      case CAPS_LOCK:
        handle_caps_lock();
        break;
      case ALT:
        handle_alt();
        break;
      default:
        handle_char(scancode);
    }
  } else {
    switch(scancode & REMOVE_SIG_BIT){ // handles keyboard releases
      case LEFT_SHIFT:
        keyboard_status = keyboard_status & DISABLE_SHIFT;
        break;
      case RIGHT_SHIFT:
        keyboard_status = keyboard_status & DISABLE_SHIFT;
        break;
      case CTRL:
        keyboard_status = keyboard_status & DISABLE_CTRL;
        break;
      case ALT:
        keyboard_status = keyboard_status & DISABLE_ALT;
        break;
      case ENTER:
        enter_pressed = ZERO;
        break;
    }

  }
  set_cursor();
  sti();

}

/*dummy function. To be populated later*/
void handle_keyboard(void)
{
      send_eoi(KEYBOARD_INTERRUPT);
}
