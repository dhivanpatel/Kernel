#ifndef KEYBOARD_H
#define KEYBOARD_H

#define ON                 1
#define ZERO               0

#define USE_SCREEN_VARS    1

#define KEYBOARD_INTERRUPT 1
#define KEYBOARD_PORT      0x60
#define KEYBOARD_RELEASE   0x80
#define KEYBOARD_VEC       0x21
#define SCANCODE_SIZE      128
#define MAX_IDX_LINE       79
#define NUM_ROWS           25
#define NEW_LINE_KB          '\n'
#define EMPTY_CHAR         ' '

#define CLEAR_SCREEN      38

#define SPECIAL_CHARS     14
#define SPEC1             26
#define SPEC2             27
#define SPEC3             40
#define SPEC4             41
#define SPEC5             43
#define SPEC6             51
#define SPEC7             52
#define SPEC8             53
#define SPEC9             55

#define F1                59
#define F2                60
#define F3                61

#define ENTER             28
#define LEFT_SHIFT        42
#define RIGHT_SHIFT       54
#define CTRL              29
#define BACKSPACE         14
#define CAPS_LOCK         58
#define ALT               56 //for terminal switching

#define REMOVE_SIG_BIT    0x7F

#define CAPS_MASK         0x0F00
#define SHIFT_MASK        0x00F0
#define CTRL_MASK         0x000F
#define ALT_MASK          0xF000 //for terminal switching

#define DISABLE_CAPS      0xF0FF
#define DISABLE_SHIFT     0xFF0F
#define DISABLE_CTRL      0xFFF0
#define DISABLE_ALT       0x0FFF //for terminal switching

#define LINE_START_IDX    7

char key_buf[SCANCODE_SIZE];
int enter_pressed;
int key_buf_idx;
int line_idx;
int keyboard_status; // CAPS , SHIFT, CTRL, ALT


extern void test_handle_keyboard(void);
extern void handle_keyboard(void);
extern void keyboard_init();
extern void kb_reset();

#endif
