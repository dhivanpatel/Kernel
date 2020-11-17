#include "paging.h"

#define R_W_BIT                 0x02    // read write enable
#define PRESENT_BIT             0x01    // present in physical memory
#define NOT_PRESENT             0x00    // not present in physical memory
#define GLOBAL_BIT              0x100   // global bit for kernel memory
#define PAGE_SIZE_BIT           0x80    // set page size to 4 MB
#define KERNEL_MEMORY_START     0x400000  //the start of kernel at 4 MB

#define USER_BIT                0x04
#define CACHE_BIT               0x10     // cacheing bit

#define VID_MEMORY_START        0xB8000
#define T0_ST                   0xB9000
#define T1_ST                   0xBA000
#define T2_ST                   0xBB000


#define VID_MEM_PTABLE_INDEX    0xB8
#define T0_PT_INDEX             0xB9
#define T1_PT_INDEX             0xBA
#define T2_PT_INDEX             0xBB
#define PT_INDEX_MASK           0x003FF000
#define PT_INDEX_SHIFT          12

#define VID_BACKLOG_CONST       0x01000
#define VID_BACKLOG_INDEX       0x01

/*enable_paging
 * set up CR0, enable the MSB to enable paging
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: none
 */
 //0x80000001 allows us to set the most significant bit and the LSB, which
 // enable paging and protected mode respectively
void enable_paging(){
    asm volatile(
      "movl %%cr0, %%eax \n"
      "orl $0x80000001, %%eax \n"
      "movl %%eax, %%cr0 \n"
      :
      :
      :"%eax"
    );
}

/*flush_tlb
 * Reloads cr3 to flush the TLB
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: none
 */
void flush_tlb(){
    asm volatile(
      "movl %%cr3, %%eax \n"
      "movl %%eax, %%cr3 \n"
      :
      :
      :"%eax"
    );
}



/*load_directory
 * set up CR3, load pagedirectory address into CR3
 *INPUTS: the address of the beginign of the page directory
 *OUTPUTS: none
 *RETURN VALUE: none
 */
void load_directory(address){
    asm volatile(
      "movl %0, %%eax \n"
      "movl %%eax, %%cr3 \n"
      :
      :"r"(address)
      :"%eax"

    );
}

/*extension
 * set up CR4 for paging extension 4 MiB pages
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: none
 */
 //0x00000010 allows us to set the 5th bit from the right, which is the PSE
void extension(){
    asm volatile(
      "movl %%cr4, %%eax \n"
      "orl $0x00000010, %%eax \n"
      "movl %%eax, %%cr4 \n"
      :
      :
      :"%eax"
    );
}



/*paging_init
 * initializes paging indirection, specifically for video memory and kernel
 *INPUTS: none
 *OUTPUTS: none
 *RETURN VALUE: 0 on success and -1 on failure
 */
int32_t paging_init(){

  int i;

  /* This loop initializes the page directory and oen page table as empty, but with a read
   * write permission granted */
  for(i = 0; i < ONE_KILOBYTE; i++){
    page_directory[i] =  R_W_BIT;
    page_table_one[i] =  R_W_BIT | NOT_PRESENT;

  }




  /* The first index of our page directory, contains a page directory entry to a page table
   * which is present in physical memory later on for video memory as well as R/W permission
   */
  page_directory[0] =  ((unsigned int)page_table_one) | (R_W_BIT | PRESENT_BIT);

  /* This is where we map the kernel memory, the first index of the page_directory
   with read/write, present, Page size, and global bit set */
  page_directory[1] = KERNEL_MEMORY_START | (R_W_BIT | PRESENT_BIT | PAGE_SIZE_BIT | GLOBAL_BIT | CACHE_BIT);

  // /*Map this specific index in our page table to point to actual video memory with R/W and present enabled*/
  page_table_one[VID_MEM_PTABLE_INDEX] = VID_MEMORY_START | (R_W_BIT | PRESENT_BIT); //causing reboot
  page_table_one[T0_PT_INDEX] = T0_ST | (R_W_BIT | PRESENT_BIT);
  page_table_one[T1_PT_INDEX] = T1_ST | (R_W_BIT | PRESENT_BIT);
  page_table_one[T2_PT_INDEX] = T2_ST | (R_W_BIT | PRESENT_BIT);


  //three functions defined above, but we are modifying the control registers to enable_paging
  load_directory(page_directory);
  extension();
  enable_paging();

  return 0;
}




/*program_paging_setup
 *Used for system call execute, but sets up the page directory entry for
 * new process 4 MB page
 *INPUTS: the virtual address where the program starts, an the actual physical address
 *OUTPUTS: none
 *RETURN VALUE: 0 on success and -1 on failure
 */
int32_t program_paging_setup(uint32_t virtualPageAddress, uint32_t  physicalMemoryAddress){
  // virtual address must be a multiple of four megabyte
  if(virtualPageAddress % KERNEL_MEMORY_START){
    return -1;
  }

  // divide virtual address by four megabytes in order to get the correct index into the Page directory
  page_directory[virtualPageAddress / KERNEL_MEMORY_START] = physicalMemoryAddress | (R_W_BIT | PRESENT_BIT | PAGE_SIZE_BIT | CACHE_BIT | USER_BIT);

  return 0;
}

/*
 *assign video mapping to correct pde and pte
 * INPUTS: vidmapAddress: Address of the virtual video memory
 * OUTPUTS: 0 if successful, -1 if fail
 */
 int32_t vidmap_setup()
 {
    page_directory[USER_VID_ST / KERNEL_MEMORY_START] = ((uint32_t)(page_table_vidmap)) | (R_W_BIT | PRESENT_BIT | USER_BIT);

    //then set up the page table
    page_table_vidmap[VID_MEM_PTABLE_INDEX] = VID_MEMORY_START | (R_W_BIT | PRESENT_BIT | USER_BIT);
    return 0;
 }

 /*
  *assign video mapping to correct pde and pte
  * INPUTS: videoAddr: Address of the virtual video memory
            Note: videoAddr NEEDS to be FOUR_KB aligned
  * OUTPUTS: 0 if successful, -1 if fail
  */
 int32_t vidmap_modify(uint32_t videoAddr)
 {
   page_directory[USER_VID_ST / KERNEL_MEMORY_START] = ((uint32_t)(page_table_vidmap)) | (R_W_BIT | PRESENT_BIT | USER_BIT);

   //then set up the page table
   page_table_vidmap[VID_MEM_PTABLE_INDEX] = videoAddr | (R_W_BIT | PRESENT_BIT | USER_BIT);
   return 0;
 }



 /*
  * stores video memory of inactive terminal into a backlog
  * INPUTS: term_num---> which terminal
  * OUTPUTS: 0 if successful, -1 if fail
  */
  int32_t backlog_vidmap_setup(int term_num)    // term num 0,1,2
  {
    if(term_num < 0 || term_num > 2){
      return -1;
    }

     term_num++;
     page_directory[USER_VID_ST / KERNEL_MEMORY_START] = ((uint32_t)(page_table_vidmap)) | (R_W_BIT | PRESENT_BIT | USER_BIT);

     //then set up the page table
     page_table_vidmap[VID_MEM_PTABLE_INDEX /*+ (VID_BACKLOG_INDEX * term_num)*/] = (VID_MEMORY_START + (VID_BACKLOG_CONST * term_num)) | (R_W_BIT | PRESENT_BIT | USER_BIT);
     return 0;
  }


 /*Clear video mapping
  *Done during halt
  * INPUTS: vidmapAddress: Address of the virtual video memory
  * OUTPUTS: 0 if successful, -1 if fail
  */
 int32_t vidmap_clear(uint32_t vidmapAddress)
 {

   page_directory[vidmapAddress / KERNEL_MEMORY_START] = 0;

   //then set up the page table
   uint32_t pt_index = (vidmapAddress & PT_INDEX_MASK) >> PT_INDEX_SHIFT;
   page_table_vidmap[pt_index] = 0;
   return 0;

 }
