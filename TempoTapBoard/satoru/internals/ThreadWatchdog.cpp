// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "ThreadWatchdog.h"
#include "SatoruAssert.h"
#include "../internals/Resources.h"
#include "SatoruPort.h"

namespace satoru {

#ifdef SATORU_THREAD_WATCHDOG
void ThreadWatchdog::StartThreadWatchdog(size_t periodMs)
{
   auto& thread = *Scheduler_currentThread;
   thread.WatchdogPeriodTicks = port::MsToTicks(periodMs);
   thread.AbsoluteLastWatchdogKick = resources.tickCounter;
}

void ThreadWatchdog::KickThreadWatchdog()
{
   auto& thread = *Scheduler_currentThread;
   SatoruAssert(thread.WatchdogPeriodTicks > 0);
   thread.AbsoluteLastWatchdogKick = resources.tickCounter;
}
#endif

}
