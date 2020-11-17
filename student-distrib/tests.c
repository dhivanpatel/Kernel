#include "tests.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*Keyboard interrupt tests and
  rtc interrupt tests are done through
	the preprocessor identifier TESTING
	in setup_idt.c*/



/* test_divide_by_zero
 * Exceptions tests, to test divide by zero
  *INPUTS: none
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int test_divide_by_zero()
{
	TEST_HEADER;
	int a = 4;
	int b = 0;
	int result = PASS;
	a /= b;
	result = FAIL;
	return result;
}

/*Arbitrary function that tests page faults with different addresses
 *page fault triggered for 1st two rest fail
 *INPUTS: None
 *OUTPUTS: None
 * RET_val: None
 */
int paging_test()
{

	int result = PASS;
		int *l =(int*) 0x2cc80;//NULL//0x400000//0xb8000
		if(*l) {
				result=FAIL;
				}
		return result;
}

/* Checkpoint 2 tests */

/* terminal_test
 * Exceptions tests, to test terminal read/write
  *INPUTS: none
  * OUTPUTS: none
  * RET_val; FAIL if exc not handled; nothing if did
 */
int terminal_test(){
	char test_buf[SCANCODE_SIZE]; // creates buffer to copy into
	int result = PASS;
	int ret = terminal_read(0, test_buf, SCANCODE_SIZE); // copies key_buf into test_buf
	int i;
	printf("VERIFY BUFFER COPY: ");
	for(i = 0; i < ret; i++){ // prints contents of test_buf to verify
		putc(test_buf[i]);
	}
	printf("READ RETURN VAL: %d \n", ret); // verifies number of bytes copied
	printf("VERIFY TERMINAL WRITE: \n"); // tests terminal write with test_buf
	ret = terminal_write(0, test_buf, ret);
	ret = terminal_write(0, test_buf, ret);
	printf("WRITE RETURN VAL: %d \n", ret); // verifies number of bytes written
	return result;

}

/*Filesystem testing*/
/*test_fs_init
 * tests initialization of file system
 * INPUTS: None
 * OUTPUTS: Error messages providing detailed info
 * RETURN VALUE: result
 									PASS for success, FAIL for failure
 * SIDE EFFECTS: None
 */
int test_fs_init()
{
	TEST_HEADER;
	int result = PASS;
	boot_block_t* begin = get_root();
	//now check for parameters in begin
	if((begin -> num_entries > NUM_DENTRIES) || (begin -> num_entries == 0))
	{
		printf("num_entries not initialized properly \n");
		result = FAIL;
	}
	if((begin -> num_inodes > (NUM_DENTRIES+1)) || (begin -> num_inodes == 0))
	{
		printf("inodes not initialized properly \n");
		result = FAIL;
	}
	return result;

}

/*test_fs_dentry_read_fname
 * tests read_dentry_by_name, thereby also checking for
 		read_dentry_by_index
 * INPUTS: None
 * OUTPUTS: Error messages for more detailed info
 * RETURN VALUE: result: PASS - success, FAIL - failure
 * SIDE EFFECTS: None
 */
int test_fs_dentry_read_fname()
{
	TEST_HEADER;
	int result = PASS;
	dentry_t directory;
	read_dentry_by_name((uint8_t *)"verylargetextwithverylongname.txt", &directory);
	//so, characteristics of directory:
	//inode number = 0 //disregard
	//filename = "."
	//filetype = 1

	//verylargetextwithverylongname
	//filename = "<as above>.tx"

	if(directory.file_type != VALID_FILE)
	{
		printf("Error in reading dentry: incorrect filetype \n");
		result = FAIL;
	}
	if(directory.file_name[FILENAME_LEN-1] != 'x')
	{
		printf("Error in reading dentry: incorrect filename \n");
		printf("Filename seen: %s \n", directory.file_name);
		result = FAIL;
	}
	return result;
}

/*
 * test_fs_dentry_read_index
 * tests read_dentry_by_index
 * INPUTS: None
 * OUTPUTS: Error messages for more detailed info
 * RETURN VALUE: result: PASS - success, FAIL - failure
 * SIDE EFFECTS: None
 */
int test_fs_dentry_read_index()
{
	TEST_HEADER;
	int result = PASS;
	uint32_t test_index = 0; /*changeable index*/
	dentry_t directory;
	read_dentry_by_index(test_index, &directory);
	//so, characteristics of directory:
	//inode number = 0 //disregard
	//filename = "."
	//filetype = 1
	if(directory.file_type != DIR_TYPE) //check if a directory, change to 2 if a file, change to 0 if rtc
	{
		printf("Error in reading dentry: incorrect filetype \n");
		result = FAIL;
	}
	if(directory.file_name[0] != '.')
	{
		printf("Error in reading dentry: incorrect filename \n");
		printf("Filename seen: %s \n", directory.file_name);
		result = FAIL;
	}

	printf("Inode num: %d \n", directory.inode);
	return result;
}

int test_fs_read_data()
{
	TEST_HEADER;
	int result = PASS;
	dentry_t d_file;
	read_dentry_by_name((uint8_t *)"counter", &d_file);
	//so, characteristics of directory:

	int32_t num_read;
	uint32_t ino = d_file.inode;
	//get inode pointer
	inode_t *file_des = get_inode(ino);
	uint32_t file_size = file_des -> inode_length;
	uint8_t buf[file_size+1];
	if ((num_read = read_data(ino, 0, &buf[0], file_size)) != file_size)
	{
		printf("Only read %d bytes ... \n", num_read);
		result = FAIL;
	}
	else
	{
		buf[file_size] = (uint8_t) '\0';
		int i;
		for(i = 0; i < 79; i++)
		{
			putc((int8_t) buf[i]);
			if( (i-1) % 80 == 0)
			{
					putc((int8_t)"\n");
			}
		}
		//printf("%s \n", (int8_t *)buf);
		//terminal_write(0, &buf[0], file_size);
	}
	return result;
}

/*test_file_open_read
 * open and read any file (not rtc or directory)
 * INPUTS: None
 * RETURN VALUE: PASS or FAIL
 * OUTPUT: prints the file passed in (change in method) as screen
 * SIDE EFFECTS: prints characters onto screen
 */
int test_file_open_read()
{
	TEST_HEADER;
	int result = PASS;
	if(file_open((uint8_t *)"counter"))
	{
		result = FAIL;
	}

	uint8_t buf[TEST_BUF_SIZE];
	uint8_t err_msg[TEST_BUF_SIZE + 1] = {"FAILURE!!!"};
	//so, characteristics of directory:

	// int32_t num_read;
	// uint32_t ino = d_file.inode;
	// //get inode pointer
	// inode_t *file_des = get_inode(ino);
	// uint32_t file_size = file_des -> inode_length;
	// uint8_t buf[file_size + 1];

	int32_t num_read;
	int32_t dontcare = 0;

	while((num_read = file_read(dontcare, buf, TEST_BUF_SIZE)) != 0)
	{
		if (num_read == -1)
		{
			terminal_write(0, (uint8_t *)err_msg, 11);
			result = FAIL;
		}
		else
		{
			terminal_write(0, (uint8_t*) buf, num_read);
		}
	}
	return result;
}


/*test_print_real_file
 * prints a file at a given d_entry index, if index points
 * to an executable or a text file
 * INPUTS: d_index : dentry index to check
 * OUTPUTS: prints the file, along with some helper comments
 * RETURN VALUE: PASS, FAIL
 * SIDE EFFECTS: prints to screen
*/
int test_print_real_file(uint32_t d_index)
{
	TEST_HEADER;
	int result = PASS;
	boot_block_t* boot = get_root();
	uint32_t max_entries = boot -> num_entries;

	if(d_index > max_entries)
	{
		printf("Invalid directory index");
		result = FAIL;
		return result;
	}

	  dentry_t d_file;
	  //what else do we do??
	  //I am really not sure
	  read_dentry_by_index(d_index, &d_file);

		printf("Reading file name: ");
		int i;
		for(i = 0; i < FILENAME_LEN; i++)
		{
			printf("%c", d_file.file_name[i]);
		}
		printf("\n");

		if((d_file.file_type) == VALID_FILE)
		{
			if(file_open((uint8_t *)(d_file.file_name)))
			{
				result = FAIL;
			}
			uint8_t buf[TEST_BUF_SIZE+1];
			//so, characteristics of directory:

			// int32_t num_read;
			// uint32_t ino = d_file.inode;
			// //get inode pointer
			// inode_t *file_des = get_inode(ino);
			// uint32_t file_size = file_des -> inode_length;
			// uint8_t buf[file_size + 1];

			int32_t num_read;
			int32_t dontcare = 0;

			while((num_read = file_read(dontcare, buf, TEST_BUF_SIZE)) != 0)
			{
				if (num_read == -1)
				{
					printf("we failed to read this file index: %d \n", d_index);
					result = FAIL;
				}
				else
				{
					buf[num_read] = (uint8_t) '\0';
					printf("%s", buf);
				}
			}
		}
		else
		{
			printf("index passed does not refer to readable file \n");
			result = FAIL;
		}

	return result;
}



/* dir_read
 * function that lists the contents of the directory
 *INPUTS: None
 *OUTPUTS: None
 * RET_val: None
 * side effects: prints all files in the directory
 */
int test_dir_read()
{
	int result = PASS;
//	dentry_t d_file;
	int32_t ret;

	directory_open((uint8_t*)"ddr");
	//so, characteristics of directory:
	int8_t buf[FILENAME_LEN];	// 32 is the max filename length
	int i;
	 while(1){
		 ret = directory_read(0, (void*) buf, FILENAME_LEN);		// pass in 0 for fd cuz we havent set it up yet
		 	if(ret == -1){		// bad inputs dir_read failed
				printf("ERROR FOR READ DIR");
				break;
			}
			if(ret == 0){		// 0 bytes read
				break;
			}
			printf("File Name: ");
			for(i = 0; i < FILENAME_LEN; i++){
				printf("%c", buf[i]);
			}
			printf("\n");


	 }

	 directory_close();

	return result;
}



/* function that tests that the rtc read/write/open works
*INPUTS: None
 *OUTPUTS: None
 * RET_val: None
 * side effects: prints 1 to the screen at various speeds
 */
int rtc_read_write(){
	int i = 0;
	int32_t j = 2; //start at a frequency of 2 Hz
	int k;
	for(k=0; k<10; k++){ //Loop through 10 times for all the different frequencies
		clear(); //clear the screen for each different frequency
		i=0;
		rtc_write(0,(void*)(&j),4); //write the new frequency
		j = j*2; //Set the frequency to the next one by multiplying by 2
		while(i<(j*2)){ //loop through for twice the next frequency
			if(i == 1000){ //stop once the number of 1's hits 1000
				break;
			}
			if((i%80 == 0) && i != 0){ //If the 1's have filled the screen go to the next line
				printf("\n");
			}
			rtc_read(0, 0, 0); //test the read to wait for the interrupt
			printf("1"); //print 1
			i++;
		}
	}

 return PASS;
}

/* function that tests that the rtc open works
*INPUTS: None
 *OUTPUTS: None
 * RET_val: None
 * side effects: prints 1 to the screen at various speeds
 */
int rtc_open_test(){
	int i = 0;
	int32_t j = 32; //start at a frequency of 32 Hz
	clear(); //clear the screen
	rtc_write(0, (void*)(&j), 4); //write the frequency
	while(i<50){
		printf("1");
		rtc_read(0,0,0); //wait for the interrupt
		if(i==40){ //Part way through printing 1's call rtc open to show that it sets the frequency to 2 Hz
			rtc_open(0);
		}
		i++;
	}
	return PASS;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("div_by_zero", test_divide_by_zero());
	//TEST_OUTPUT("paging", paging_test());
	// TEST_OUTPUT("paging", paging_test());

	//test_fs_read_data();
	TEST_OUTPUT("terminal", terminal_test());
	//TEST_OUTPUT("dir_read", test_dir_read());
	//TEST_OUTPUT("file_read", test_file_open_read());
	//TEST_OUTPUT("print specific file", test_print_real_file(13)); //prints file index 13 (testprint)
	//TEST_OUTPUT("rtc read/write", rtc_read_write());
	//TEST_OUTPUT("rtc open", rtc_open_test());
	// launch your tests here
}
