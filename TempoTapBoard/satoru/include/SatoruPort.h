// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SCHEDULER_PORT_H
#define SCHEDULER_PORT_H

#include <stdint.h>
#include <stddef.h>

namespace satoru 
{
   namespace port
   {
      void Initialise();
      void StartTick();
      size_t MsToTicks(size_t ms);
      size_t TicksToMs(size_t ticks);
      size_t GetThreadStackPointer(void);
      size_t GetHandlerStackPointer(void);
   }
}

#endif
