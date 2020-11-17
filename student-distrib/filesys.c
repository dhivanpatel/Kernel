#include "filesys.h"
#include "syshelp.h"
#include "PCB.h"
//#include "../fish/ece391support.h"


//static int32_t fs_strcmp (const uint8_t* s1, const uint8_t* s2, uint32_t n);


static dentry_t d_entry; // temp dentry

static int32_t byte_offset = 0; //offset value that we use since we dont have PCB set up yet

static int32_t dir_number_dir_read = 0; // global location value for which d_entry we are on

/* filesys_init
 * initializes the file system during the boot process
 * INPUTS: root_loc: pointer to the start of the filesystem
 * OUTPUTS: None
 * RETURN VALUE: None
 * SIDE EFFECTS: initializes filesystem boot block
 */
int32_t filesys_init(uint32_t root_loc)
{
  if((void *) root_loc == NULL)
  {
    return -1;
  }
  root_block = (boot_block_t*) root_loc;
  data_blocks = (uint8_t *)(root_loc + D_BLOCK_ST);
  inode_start = (inode_t *)(root_loc + FOUR_KB);
  return 0;
}


/* read_dentry_by_name
 * copies the file information to the dentry structure pointer
 * passed in
 * to be used in syscall open I think
 * INPUTS: fname: string of file name
           dentry: location of dentry struct to copy to
 * OUTPUTS: None
 * RETURN VALUE:0 if successfully copied, -1 if failure
 * SIDE EFFECTS: modifies dentry parameter
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  //so, we have a filename
  //then, we also have a pointer to a directory
  //we return 0 or -1 right
  //when do we return -1?

  /*return -1 for invalid parameters*/
  if((dentry == NULL) || (fname == NULL))
  {
    return -1;
  }

  /* return -1 if fname is empty string*/
  if(fname[0] == (uint8_t) '\0')
  {
    return -1;
  }
  //strncmp? -- let's try using it and see what happens
  //iterate through the boot block and find out
  //which inode to access

  uint32_t i;
  for(i = 0; i < NUM_DENTRIES; i++)
  {
    //using strcmp to check equality between the file names
    if(!fs_strcmp(fname, (root_block -> dir_entries[i]).file_name, FILENAME_LEN))
    {
        //int32_t inode_num = root_block.dir_entries[i].inode;
        //uint32_t index = ++inode_num;
        return read_dentry_by_index(i, dentry);
    }
  }
  return -1;
}


/*
 * read_dentry_by_index
 * Given an index to the file_system structure in boot_block,
 * copy the contents of the directory entry into this one
 * INPUTS: index: index of the directory entry in the dentry array
 * OUTPUTS: None
 * RETURN VALUE: 0 if successfullly copied,
 *              -1 if failure
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  if((index > NUM_DENTRIES) || (dentry == NULL))
  {
    return -1;
  }
  dentry_t *copy_dentry = &((root_block -> dir_entries)[index]);

  //populate dentry
  (dentry -> file_type) = (copy_dentry -> file_type);
  (dentry -> inode) = (copy_dentry -> inode);
  //copy the contents of filename
  uint32_t i;
  for(i = 0; i < FILENAME_LEN; i++)
  {
    (dentry -> file_name)[i] = (copy_dentry -> file_name)[i];
  }
  return 0;
}

/* read_data
 * reads data from a file to a buffer of size length
 * INPUTS: inode: index node of file
           offset: offset(in bytes) from which to access file
           buf: buffer to copy contents to
           length: size of buf
 * OUTPUTS: None
 * RETURN VALUE: num_bytes_read = number of bytes read
 * SIDE EFFECTS: None
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  //index in boot_block = inode + 1 right?
  uint32_t num_bytes_read = 0;
  //so, we know the inode
  //just do inode + 1 to get the offset from boot_block
  //any thing we need to do to check the inode?

  inode_t* file_data = &(inode_start[inode]);
  //how to know if reached end of file?
  //don't worry about this. These are helper functions
  //actual file_open will handle it
  //I'm guessing we go through read_dentry_by_name,
  //then check what type of a file it is
  //I'm guessing we load our file in the process
  //control block for each array

  //figure out what data block to start at
  if(offset >= file_data -> inode_length)
  {
    return num_bytes_read;
  }
  uint32_t block_index;
  uint32_t block_offset;
  /*'cursor' to know place in file */
  uint32_t block_cursor = offset;
  //each block is four kilobytes
  //therefore, we need to take offset and divide it by 4KB

  //formula:
  //for every byte:
  //do block_cursor/4096 and to get index to block
  //to access
  //then, do block_cursor % 4096 to get the byte to access inside block
  //when to stop?
  //when block cursor has reached the inode_length
  //block_cursor represents the 'location' index of the byte
  //to read in this virtual mapping of data
  //what else do we need to work on?

  while(num_bytes_read < length)
  {

    if(block_cursor >= (file_data -> inode_length))
    {
      return num_bytes_read;
    }
    /*access the right block*/
    block_index = file_data -> datablock_nums[block_cursor/ FOUR_KB];
    block_offset = block_cursor % FOUR_KB;
    *buf = *(data_blocks + (block_index * FOUR_KB) + block_offset);

    buf++;
    num_bytes_read++;
    block_cursor++;
  }
  return (int32_t) num_bytes_read;
}


/*get_root
 *test function to check initialization, returns
 * root_block for use in test functions
 * INPUTS: None
 * OUTPUTS: None
 * RETURN VALUE: root_block: pointer to boot_block of system
 * SIDE EFFECTS: None
 */
boot_block_t* get_root()
{
  return root_block;
}

/*get_inode
 * test function to return inode struct pointer
 * of file to check
 * INPUTS: inode - inode to use
 * OUTPUTS: None
 * RETURN VALUE: pointer to inode_struct to access
                 NULL if invalid
 * SIDE EFFECTS: None
 */
 inode_t* get_inode(uint32_t inode)
 {
   return (inode_t*)(&inode_start[inode]);
 }


/* fs_strcmp
 * function to compare strings
 * INPUTS: s1 - first string to compare to
           s2 - second string to compare to
           n  - number of characters for which to check
 * RETURN VALUE: 0 if equal, > 0 if s1 after s2, < 0 otherwise
 * OUTPUTS: None
 * SIDE EFFECTS: None
 */
int32_t
fs_strcmp (const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
	return 0;
    while (*s1 == *s2) {
        if (*s1 == CLEAR_CHAR || --n == 0)
	       return 0;
	      s1++;
	      s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

/*file_write
 * write to file--not implemented
 * INPUTS: fd pointer, buffer of what we want to write, number of bytes to write
 * OUTPUTS: None
 * RETURN VALUE: -1 always cuz we dont implemented
 * SIDE EFFECTS: None
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){

  return -1;
}



/*file_read
 * read from file
 * INPUTS: fd pointer, buffer of file name, number of bytes to read
 * OUTPUTS: None
 * RETURN VALUE: -1 if read data fails, otherwise number of bytes read
 * SIDE EFFECTS: None
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
  PCB_t* cur = (PCB_t *)((uint32_t)(&fd) & PCB_MASK);
  uint32_t bytes_offset = 0;

  bytes_offset = read_data((cur -> file_des_array[fd]).inode, (cur -> file_des_array[fd]).file_position, buf, nbytes);

  if(bytes_offset != -1)
   (cur -> file_des_array[fd]).file_position += bytes_offset;

  return bytes_offset;
}



/*file_open
 * opens a file
 * INPUTS: filename array
 * OUTPUTS: None
 * RETURN VALUE: 0
 * SIDE EFFECTS: populates a d_entry structure we can use later on
 */
int32_t file_open(const uint8_t* filename){
  // populate our global dentry for later use
  read_dentry_by_name(filename, &d_entry);

  // reset our byte offset value for later use
  byte_offset = 0;

  return 0;
}


/*file_open
 * closes a file
 * INPUTS: file descriptor pointer
 * OUTPUTS: None
 * RETURN VALUE: 0
 * SIDE EFFECTS: clears some global structures/vars
 */
int32_t file_close(int32_t fd){
  // reset byte offset
  byte_offset = 0;
  int i;


  // clear our golbal dentry and fill it with garbage values so we get errors if we do not intially open it
  for(i = 0; i < FILENAME_LEN; i++){
    d_entry.file_name[i] = '#';
  }
    d_entry.file_type = 50;  // garbage file type value cuz we dont go above filetype 3
    d_entry.inode = 0;      // just make it zero now, will change later if we open another file
  return 0;
}



/*directory_open
 * opens a directory
 * INPUTS: filename array
 * OUTPUTS: None
 * RETURN VALUE: 0
 * SIDE EFFECTS: resets global variables evrytime we open a directory
 */
int32_t directory_open(const uint8_t* filename){
  // right now we dont do anything

  return 0;
}


/*directory_close
 * closes a directory
 * INPUTS: none
 * OUTPUTS: None
 * RETURN VALUE: 0
 * SIDE EFFECTS: closes directory
 */
int32_t directory_close(){
  return 0;
}


/*directory_write
 * writes to  a directory
 * INPUTS: none
 * OUTPUTS: None
 * RETURN VALUE: -1 since we dont implement it
 * SIDE EFFECTS: none
 */
int32_t directory_write(){
  return -1;
}


/*directory_read
 * reads through a directory of files
 * INPUTS: fd pointer, buffer we copy to, number of bytes
 * OUTPUTS: None
 * RETURN VALUE: -1 on bad inputs, number of bytes read otherwise
 * SIDE EFFECTS: reads directory data and copies to a buffer
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){



  // check for bad inputs
  if(nbytes < 0) return -1;
  if(buf == NULL) return -1;
  if(nbytes == 0) return 0;


  PCB_t* curr = (PCB_t *)(((uint32_t)&fd) & PCB_MASK); // get our pcb

  dir_number_dir_read = (curr -> file_des_array[fd]).file_position;
  // check if we have seen all the directory entries, if yes we just change it back to 0 for later use aka reset it
  if(dir_number_dir_read == root_block-> num_entries){
    //dir_number_dir_read = 0;
    return 0;
  }

  int32_t bytes = 0;    // how many bytes have been read so far
  int j;
  dentry_t temp_dentry;
  uint8_t* buffer = (uint8_t*)buf;  // cast the void buffer

  for(j = 0; j < FILENAME_LEN; j++){
    buffer[j] = CLEAR_CHAR; // clear buffer from any prevoious uses
  }

  // populate our temp dentry with the file name and any other info
  bytes = read_dentry_by_index(dir_number_dir_read, &temp_dentry);

  //bad read?
  if(bytes == -1){
    return -1;
  }

  // copy however many bytes we want to read to the buffer from the file name
  for(j = 0; j < nbytes; j++, bytes++)
    {
      buffer[j] = temp_dentry.file_name[j];
    }


  // increment the dentry we are at
  dir_number_dir_read++;
  (curr -> file_des_array[fd]).file_position = dir_number_dir_read;
  return bytes;

}


/*
 * load_exec_to_page
 * loads an executable to page, given an inode
 * called by execute system call
 * do you want to pass just filename?
 * maybe makes life easier
 * INPUTS: inode: Inode of file to load
 * OUTPUTS: None
 * RETURN VALUE: None
 * SIDE EFFECTS: loads file into virtual memory
 */
 void load_exec_to_page(uint32_t inode)
 {

   uint8_t* load_loc = (uint8_t *)(CODE_START_ADDR);
   inode_t* file_to_load = &inode_start[inode];

   uint32_t file_size = file_to_load -> inode_length;

   read_data(inode, 0, load_loc, file_size);
 }
