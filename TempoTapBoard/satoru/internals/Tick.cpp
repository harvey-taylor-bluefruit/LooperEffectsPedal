// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Tick.h"
#include "TimerEntry.h"
#include "Satoru.h"
#include "StackChecks.h"
#include "Resources.h"
#include "CriticalSection.h"
#include "Watchdog.h"
#include "SatoruPort.h"
#include "SatoruAssert.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>

namespace satoru
{

static size_t absoluteNextWakeUp = 0U;

void WakeUpCycle()
{
   CriticalSection section;
   size_t minRelativeTimeToWakeUp = UINT32_MAX;
   for (size_t i = 1U; i < resources.createdThreadCount; i++) {
      Thread *thread = &resources.threadPool[i];

      if (thread->Status != ThreadStatus_Suspended) {
         continue;
      }

      if (thread->IsAsleep && (thread->AbsoluteTicksToActive == resources.tickCounter)) {
         thread->Status = ThreadStatus_Active;
         thread->IsAsleep = false;
      } else if (thread->IsAsleep) {
         size_t relativeTimeToWakeUp = thread->AbsoluteTicksToActive - resources.tickCounter;

         if (relativeTimeToWakeUp < minRelativeTimeToWakeUp) {
            minRelativeTimeToWakeUp = relativeTimeToWakeUp;
            absoluteNextWakeUp = thread->AbsoluteTicksToActive;
         }
      }
   }
}

#ifdef SATORU_WATCHDOG
void ServiceWatchdog()
{
   static bool watchdogStarted = false;
   if (!watchdogStarted) {
      Watchdog::Start();
      watchdogStarted = true;
   }
   static size_t lastKick = resources.tickCounter;
   static const size_t SafeKickPeriodTicks = port::MsToTicks(config::SatoruWatchdogPeriodMs / 3);
   if ((resources.tickCounter - lastKick) > SafeKickPeriodTicks)
   {
      lastKick = resources.tickCounter;
      Watchdog::Kick();
   }
}
#endif

#ifdef SATORU_THREAD_WATCHDOG
namespace {
   const size_t IdleServicePeriodTicks = 1000u;
   size_t ticksUntilService = IdleServicePeriodTicks;
   size_t ticksAtLastService = 0u;
}

void ServiceThreadWatchdog()
{
   if ((resources.tickCounter - ticksAtLastService) < ticksUntilService) {
      return;
   }

   ticksAtLastService = resources.tickCounter;
   ticksUntilService = IdleServicePeriodTicks;
   for (size_t i = 1u; i < resources.createdThreadCount; i++) {
      Thread *thread = &resources.threadPool[i];
      if (thread->WatchdogPeriodTicks > 0u
            && (resources.tickCounter - thread->AbsoluteLastWatchdogKick) >= thread->WatchdogPeriodTicks)
      {
         const size_t errorMessageLength = 32u;
         char errorMessage[errorMessageLength];
         snprintf(errorMessage, errorMessageLength, "WATCHDOG RESET: %s thread.\r\n", thread->Name);
         FATAL(errorMessage);
      } else if (thread->WatchdogPeriodTicks > 0u) {
         uint32_t threadTicksUntilService = thread->WatchdogPeriodTicks - (resources.tickCounter - thread->AbsoluteLastWatchdogKick);
         ticksUntilService = std::min(ticksUntilService, threadTicksUntilService);
      }
   }
}
#endif

extern "C" void TickHandler(void)
{
   bool switchRequired = false;

   resources.tickCounter++;

#ifdef SATORU_WATCHDOG
   ServiceWatchdog();
#endif

#ifdef SATORU_THREAD_WATCHDOG
   ServiceThreadWatchdog();
#endif

   if (resources.tickCounter == absoluteNextWakeUp) {
      WakeUpCycle();
      switchRequired = true;
   }

   while (resources.firstActiveTimer && (resources.firstActiveTimer->absoluteTimeTicks == resources.tickCounter)) {
      resources.firstActiveTimer = resources.firstActiveTimer->next;
      resources.timerThreadSemaphore.Give();
   }

   if(switchRequired) {
      core::SelectNextThreadToRun();
   } else {
      // optimisation: stack integrity should be checked before selecting 
      // next thread to run!
      CheckStackIntegrity(); 
   }
}

}
