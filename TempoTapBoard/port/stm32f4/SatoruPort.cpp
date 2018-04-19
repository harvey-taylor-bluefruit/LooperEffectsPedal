#include "SatoruPort.h"
#include "Satoru.h"
#include "../core/Tick.h"
#include "Clocks.h"
#include "InterruptPriorities.h"

#include "stm32f4xx.h"
#include "core_cmFunc.h"

using namespace ::satoru;
using namespace ::sandbox::STM32F4Drivers;

namespace {
   /* From STM32F3 and STM32F4 Series Cortex ® -M4 programming manual PM0214 rev4,
   section 4.5.5, SysTick design hints and tips
   The SysTick counter reload and current value are undefined at reset, the correct
   initialization sequence for the SysTick counter is:
   1. Program reload value.
   2.  Clear current value.
   3.  Program Control and Status register

   From section 4.5.2: SysTick reload value register (STK_LOAD)
   The RELOAD value is calculated according to its use:
   l To generate a multi-shot timer with a period of N processor clock cycles, use a RELOAD
   value of N-1. For example, if the SysTick interrupt is required every 100 clock pulses, set
   RELOAD to 9
   */

   const uint32_t ReloadValueFor1MsInterrupt = (Clocks::SystemClockFrequencyHz / 1000U) - 1U; // With processor clock as source

   void SysTickInit()
   {
      SysTick->LOAD  = ReloadValueFor1MsInterrupt;
      NVIC_SetPriority (SysTick_IRQn, InterruptPriorities::SysTickTimer);
      SysTick->VAL   = 0UL;
      SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | // uses processor clock
                       SysTick_CTRL_TICKINT_Msk   |
                       SysTick_CTRL_ENABLE_Msk;
   }

   void CoreInit()
   {
      SCB->CCR |= SCB_CCR_STKALIGN_Msk; // force double word alignment so that LDRD/STRD instructions can be used
      NVIC_SetPriority(PendSV_IRQn, InterruptPriorities::LowestPriority); // PendSV is used for context switching and must be lowest priority
   }
}

void satoru::port::Initialise()
{
   CoreInit();
   SysTickInit();
}

uint32_t satoru::port::MsToTicks(uint32_t ms)
{
   return ms;  // 1ms tick
}

uint32_t satoru::port::GetThreadStackPointer()
{
   return __get_PSP();
}

uint32_t satoru::port::GetHandlerStackPointer()
{
   return __get_MSP();
}

extern "C" void SysTick_Handler(void)
{
   TickHandler();
}
