// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Watchdog.h"
#include "SatoruConfig.h"
#include "stm32f7xx.h"
#include "SatoruAssert.h"
#include "../internals/Resources.h"
#include "SatoruPort.h"

namespace satoru 
{

namespace 
{
   static const size_t aKickCommand = 0xAAAA;
   static const size_t aEnableWriteCommand = 0x5555;
   static const size_t aStartCommand = 0xCCCC;
}

bool Watchdog::WasResetByWatchdog()
{
   bool wasReset = RCC->CSR & RCC_CSR_IWDGRSTF_Msk;
   RCC->CSR |= RCC_CSR_RMVF_Msk; // Clears the flag
   return wasReset;
}

void Watchdog::Start()
{
#ifndef NDEBUG
   DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP; // freeze watchdog when halted for debug
#endif

   while (IWDG->SR & IWDG_SR_PVU_Msk)
   {} // Wait for the divider to stabilise

   IWDG->KR = aEnableWriteCommand; // Enable write access
   IWDG->PR = 0x03;   // 32 divider to obtain 1KHz clock

   while (IWDG->SR & IWDG_SR_RVU_Msk)
   {} // Wait for the reload value to stabilise

   IWDG->RLR = config::SatoruWatchdogPeriodMs; // 1KHz clock so 1 tick = 1 ms.
   IWDG->KR = aStartCommand; // Starts the watchdog
}

void Watchdog::Kick()
{
   static const size_t KickCommand = aKickCommand;
   IWDG->KR = KickCommand;
}

}
