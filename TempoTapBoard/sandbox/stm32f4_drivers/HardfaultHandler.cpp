#include "stm32f4xx.h"

namespace stm32_drivers
{

namespace HardfaultRegisters
{
   volatile uint32_t r0;
   volatile uint32_t r1;
   volatile uint32_t r2;
   volatile uint32_t r3;
   volatile uint32_t r12;
   volatile uint32_t lr; /* Link register. */
   volatile uint32_t pc; /* Program counter. */
   volatile uint32_t psr;/* Program status register. */
}

extern "C"
{
#ifdef __GNUC__
   __attribute__((naked)) void HardFault_Handler()
#else
   __task void HardFault_Handler()
#endif
   {
      static volatile uint32_t* faultStackPtr = 0;
      asm
      (
         " tst lr, #4               \n"
         " ite eq                   \n"
         " mrseq %[addr], msp       \n"
         " mrsne %[addr], psp       \n"
         : [addr]"=r"(faultStackPtr)
      );

      HardfaultRegisters::r0 = faultStackPtr[0];
      HardfaultRegisters::r1 = faultStackPtr[1];
      HardfaultRegisters::r2 = faultStackPtr[2];
      HardfaultRegisters::r3 = faultStackPtr[3];
      HardfaultRegisters::r12 = faultStackPtr[4];
      HardfaultRegisters::lr = faultStackPtr[5];
      HardfaultRegisters::pc = faultStackPtr[6];
      HardfaultRegisters::psr = faultStackPtr[7];

      for (;;);
   }
}

}
