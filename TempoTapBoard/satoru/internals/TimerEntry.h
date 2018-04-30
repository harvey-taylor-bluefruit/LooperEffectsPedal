// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef TIMER_ENTRY_H
#define TIMER_ENTRY_H

#include "CountingSemaphore.h"

#include <stdint.h>

namespace satoru
{

typedef struct TimerEntry {
   void (*callback)(void);
   size_t absoluteTimeTicks;
   struct TimerEntry* next;
   struct TimerEntry* previous;
   size_t repeatPeriod;

   TimerEntry():
      callback(0),
      absoluteTimeTicks(0),
      next(0),
      previous(0),
      repeatPeriod(0)
   {}
} TimerEntry;

}

#endif
