// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

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
      // Enable watchdog by defining SATORU_WATCHDOG
      static const uint16_t SatoruWatchdogPeriodMs = 1000u;
      // Only 11 bits with the desired precision
      static_assert((SatoruWatchdogPeriodMs >> 11) == 0, 
                    "Only 11 bits with the desired precision");   
      static const uint32_t KernelThreadStackSize = 256U;
      static const uint32_t HandlerStackFillSize = 200U;
      static const uint32_t TimerThreadStackSize = 512U;
   }
}

#endif
