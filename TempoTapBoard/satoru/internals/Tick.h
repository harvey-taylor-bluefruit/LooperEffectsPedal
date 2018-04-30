// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef TICK_H
#define TICK_H

#include "Thread.h"
#include "SatoruConfig.h"

#include <stdint.h>

namespace satoru 
{
   extern "C" void TickHandler(void);
   void WakeUpCycle();
}

#endif
