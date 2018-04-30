// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "SatoruPort.h"
#include "Satoru.h"
#include "../internals/Tick.h"
#include "Clocks.h"
#include "InterruptPriorities.h"

#include "stm32f7xx.h"
#include "core_cmFunc.h"

using namespace ::satoru;

/* From ARM Cortex M7 Devices Generic User Guide (DUI 0646A (ID042815))
section 4.4.5, SysTick design hints and tips
The SysTick counter reload and current value are undefined at reset, the correct
initialization sequence for the SysTick counter is:
1. Program reload value.
2.  Clear current value.
3.  Program Control and Status register

From section 4.4.2: SysTick reload value register (STK_LOAD)
The RELOAD value is calculated according to its use:
l To generate a multi-shot timer with a period of N processor clock cycles, use a RELOAD
value of N-1. For example, if the SysTick interrupt is required every 100 clock pulses, set
RELOAD to 9
*/

static const size_t ReloadValueFor1MsInterrupt = (::stm32_drivers::Clocks::SystemClockFrequency_Hz / 1000U) - 1U; // With processor clock as source;

void satoru::port::StartTick()
{
   SysTick->LOAD  = ReloadValueFor1MsInterrupt;
   NVIC_SetPriority (SysTick_IRQn, InterruptPriorities::SysTickTimer);
   SysTick->VAL   = 0UL;
   SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | // uses processor clock
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

void satoru::port::Initialise()
{
   // PendSV is used for context switching and must be lowest priority
   NVIC_SetPriority(PendSV_IRQn, InterruptPriorities::LowestPriority); 
}

size_t satoru::port::MsToTicks(size_t ms)
{
   return ms;  // 1ms tick
}

size_t satoru::port::TicksToMs(size_t ticks)
{
   return ticks;  // 1ms tick
}

size_t satoru::port::GetThreadStackPointer()
{
   return __get_PSP();
}

size_t satoru::port::GetHandlerStackPointer()
{
   return __get_MSP();
}

