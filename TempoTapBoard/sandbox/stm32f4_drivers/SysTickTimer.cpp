#include "SysTickTimer.h"
#include "Clocks.h"
#include "SatoruAssert.h"
#include "Satoru.h"

#include "stm32f4xx.h"

namespace stm32_drivers { 
namespace SysTickTimer {

   void BlockingDelayUs(size_t delayUs)
   {
      SatoruAssert(!satoru::core::IsRunning());
      const uint32_t TicksToCount =
         (delayUs * (Clocks::SystemClockFrequencyHz / 1000000U)) - 1U;  // processor frequency is a multiple of 1MHz, so no precision loss here

      SatoruAssert(TicksToCount < (1 << 24)); // SysTick Reload register is 24bit only!
      SysTick->LOAD = TicksToCount;
      SysTick->VAL = 0UL;
      SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // uses processor clock
                      SysTick_CTRL_ENABLE_Msk;

      SysTick->LOAD = 0U; // disables counter after overflow

      while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
         __NOP();
      }
   }

   void BlockingDelayMs(size_t delayMs)
   {
      for (uint32_t i = 0; i < delayMs; i++) {
         BlockingDelayUs(1000U);
      }
   }
}
}
