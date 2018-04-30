// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

namespace satoru
{

class Watchdog
{
public:
   static void Start();
   static void Kick();
   static bool WasResetByWatchdog();
};

}

#endif
