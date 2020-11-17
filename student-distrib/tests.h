#ifndef TESTS_H
#define TESTS_H


#define TEST_BUF_SIZE 10
#define VALID_FILE 2
#define DIR_TYPE 1
// test launcher
void launch_tests();

//keyboard test
extern void test_handle_kb_interrupt(void);
//rtc test
extern void test_handle_rtc_interrupt(void);

#endif /* TESTS_H */
