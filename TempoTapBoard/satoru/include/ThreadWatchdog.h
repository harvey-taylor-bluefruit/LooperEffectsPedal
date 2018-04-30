// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef THREAD_WATCHDOG_H
#define THREAD_WATCHDOG_H

#include <stdint.h>

namespace satoru
{
  
#ifdef SATORU_THREAD_WATCHDOG
class ThreadWatchdog
{
public:
   static void StartThreadWatchdog(size_t periodMs);
   static void KickThreadWatchdog();
};
#endif

}

#endif
