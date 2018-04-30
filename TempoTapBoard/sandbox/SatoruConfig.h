#ifndef SATORU_CONFIG_H
#define SATORU_CONFIG_H

#include <stdint.h>

// This sample file is not referenced anywhere. You must provide a "SatoruConfig.h"
// file in this format to be referenced by Satoru files.
namespace satoru
{
   namespace config
   {
      static const uint8_t MaxNumberOfThreads = 10;
      static const uint32_t MaximumActiveTimers = 32u;
      static const uint16_t SatoruWatchdogPeriodMs = 2000u; // Enable watchdog by defining SATORU_WATCHDOG
      static_assert((SatoruWatchdogPeriodMs >> 11) == 0, 
                     "Watchdog period must be 11 bits only");
      static const uint32_t KernelThreadStackSize = 256U;
      static const uint32_t HandlerStackMaxSafeUsage = 0x300U; // should be less than linker-defined stack size
      static const uint32_t TimerThreadStackSize = 512U;

   }
}

#endif
