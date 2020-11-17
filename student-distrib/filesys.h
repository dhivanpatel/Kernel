#ifndef FILE_SYS_H
#define FILE_SYS_H

/*Set of files to maintain a filesystem*/
/*structures to handle file systems*/
/*Need a boot_block struct*/
/*Then each inode block that is a file needs
 * it's own struct*/

//#define ASM 1
#include "types.h"

#define FILENAME_LEN    32
#define RESERVED_BOOT   52 /*number of 4 bytes to reserve in block*/
#define RESERVED_DENTRY 24 /*number of reserved bytes in our dentry structure*/
#define NUM_DENTRIES    63 /*number of dentries inside the boot block*/
#define NUM_INODE_DATA  1023
#define FOUR_KB         4096
#define D_BLOCK_ST      FOUR_KB*(NUM_DENTRIES + 2)/*offset for when data blocks start*/
//#define INODE_ST


#define RTC_FILE        0
#define DIRECTORY       1
#define VALID_FILE      2




#define CODE_START_ADDR 0x8048000

#define CLEAR_CHAR      '\0'



typedef struct dentry{
uint8_t file_name[FILENAME_LEN];
uint32_t file_type;
uint32_t inode;
uint8_t reserved[RESERVED_DENTRY];
}dentry_t;

typedef struct boot_block{
  uint32_t num_entries;
  uint32_t num_inodes;
  uint32_t num_datablocks;
  uint8_t reserved[RESERVED_BOOT];
  dentry_t dir_entries[NUM_DENTRIES];
}boot_block_t;

typedef struct inode{
  uint32_t inode_length;
  uint32_t datablock_nums[NUM_INODE_DATA];
}inode_t;

// //need one more structure i think:
// //so need to figure out how many data blocks we have,
// //right?
// typedef struct filesys{
//
//   boot_block_t boot_block;
//   inode inode_arr[NUM_DENTRIES];
//
// }


boot_block_t* root_block;
inode_t* inode_start; //start of the inodes
uint8_t* data_blocks; //actual data blocks







int32_t filesys_init(uint32_t root_loc);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


/*read/open/write functions*/


//funciton that writes to a file, but we have a read only filesystem so we dont need it
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

//read certain bytes from a file
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

//open a file
int32_t file_open(const uint8_t* filename);

//close a file
int32_t file_close(int32_t fd);



int32_t directory_open(const uint8_t *filename);

int32_t directory_close();

int32_t directory_write();

int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

int32_t fs_strcmp (const uint8_t* s1, const uint8_t* s2, uint32_t n);

void load_exec_to_page(uint32_t inode);

/*Test functions for checking file system*/
boot_block_t* get_root();
inode_t* get_inode(uint32_t inode);

#endif
