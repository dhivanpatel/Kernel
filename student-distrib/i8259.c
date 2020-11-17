/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

  unsigned long flags;

  /*Uni processor, so don't need a lock*/
  cli_and_save(flags);
  /*Mask bits of master and slave - use data port*/
  master_mask = inb(MASTER_DATA_PORT);
  slave_mask = inb(SLAVE_DATA_PORT);
  outb(MASK_DATA, MASTER_DATA_PORT);
  outb(MASK_DATA, SLAVE_DATA_PORT);
  /*Initialize master*/
  outb(ICW1, MASTER_8259_PORT);
  outb(ICW2_MASTER, MASTER_DATA_PORT);
  outb(ICW3_MASTER, MASTER_DATA_PORT);
  outb(ICW4, MASTER_DATA_PORT);
  /*Initialize slave*/
  outb(ICW1, SLAVE_8259_PORT);
  outb(ICW2_SLAVE, SLAVE_DATA_PORT);
  outb(ICW3_MASTER, SLAVE_DATA_PORT);
  outb(ICW4, SLAVE_DATA_PORT);
  /*Restore flags on the data port*/
  outb(master_mask, MASTER_DATA_PORT);
  outb(slave_mask, SLAVE_DATA_PORT);


  sti();
  restore_flags(flags);

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

  uint8_t irq_mask = (uint8_t)(~(1 << (irq_num % 8)));

  if(irq_num >= 8)
  {
    slave_mask &= irq_mask;
    outb(slave_mask, SLAVE_DATA_PORT);
  }
  else
  {
    master_mask &= irq_mask;
    outb(master_mask, MASTER_DATA_PORT);
  }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  uint8_t irq_mask = (uint8_t)(1 << (irq_num % 8));

  if(irq_num >= 8)
  {
    slave_mask |= irq_mask;
    outb(slave_mask, SLAVE_DATA_PORT);
  }
  else
  {
    master_mask |= irq_mask;
    outb(master_mask, MASTER_DATA_PORT);
  }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
  uint32_t cascade_irq = 2;
  if(irq_num >= 8)
  {
    outb((EOI | (irq_num - 8)), SLAVE_8259_PORT);
    outb((EOI | cascade_irq), MASTER_8259_PORT);
  }
  else
  {
    outb((EOI | irq_num), MASTER_8259_PORT);
  }
}
