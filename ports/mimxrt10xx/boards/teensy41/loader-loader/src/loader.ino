#include "tinyuf2-binary.h"

static inline void __set_MSP(uint32_t topOfMainStack)
{
    asm volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}

static inline void __set_PSP(uint32_t topOfProcStack)
{
    asm volatile ("MSR psp, %0" : : "r" (topOfProcStack) : );
}


void board_app_jump(void)
{
    void *tinyuf2_loadaddr = (void *)0x1000;
    void *tinyuf2_jumpaddr = (void *)0x2000;
    uint8_t *tinyuf2_vectors  = (uint8_t *)0x2000;
    uint8_t stack_pointer     = 0; // hmm, does this matter?  

    __disable_irq();
    memcpy(tinyuf2_loadaddr, tinyuf2_binary, tinyuf2_length);
    /* switch the memory configuration to the default */

    // TEENSY sets GPR14 to 0x00AA0000.  not sure why the TEENSY startup
    // code sets this value. The bits set seem to be RESERVED
    // in the docs.
    IOMUXC_GPR_GPR14 = 0x00000000; 

    // Set flexram settings back to fuse bank settings.
    IOMUXC_GPR_GPR16 = 0x00200003; 
    // This shouldn't matter, but set back to defaults anyway
    IOMUXC_GPR_GPR17 = 0x00000000; 

    // Doing this because the teensy does it.  must be a reason...
    __asm__ volatile("dsb":::"memory");
    __asm__ volatile("isb":::"memory");
  
    /* switch exception handlers to the application */
    SCB_VTOR = (uint32_t) tinyuf2_vectors;

    // Set stack pointer
    __set_MSP(stack_pointer);
    __set_PSP(stack_pointer);

    // Jump to Application Entry
    asm("bx %0" ::"r"(tinyuf2_jumpaddr));
}

void setup(void)
{
    board_app_jump();
}

void loop(void)
{
}

extern "C" void usb_isr(void);
extern "C" void usb_isr(void)
{

}
