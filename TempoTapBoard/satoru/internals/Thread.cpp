// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Thread.h"
#include "Resources.h"
#include "Tick.h"
#include "StackChecks.h"
#include "Context.h"
#include "CriticalSection.h"

namespace satoru {

namespace core {

void ThreadSleepTicks(size_t ticks)
{
   { CriticalSection section;
      Scheduler_currentThread->AbsoluteTicksToActive = resources.tickCounter + ticks;
      Scheduler_currentThread->Status = ThreadStatus_Suspended;
      Scheduler_currentThread->IsAsleep = true;
      WakeUpCycle();
   }

   SelectNextThreadToRun();
}

void ThreadTerminate()
{
   Scheduler_currentThread->Status = ThreadStatus_Free;
   WakeUpCycle();
   SelectNextThreadToRun();
}

void SelectNextThreadToRun()
{
   CriticalSection section;

   Thread *thread = Scheduler_currentThread;
   Thread *maxPriorityThread = Scheduler_KernelThread;
   uint8_t maxPriority = Resources::KernelThreadPriority;
   for (size_t i = 0U; i < resources.createdThreadCount; i++ ) {
      thread++;

      if(thread > &resources.threadPool[resources.createdThreadCount]) {
         thread = &resources.threadPool[0];
      }

      if(thread->Status != ThreadStatus_Active) {
         continue;
      }

      if(thread->Priority == Resources::HighestPriority) { // optimisation
         maxPriorityThread = thread;
         break;
      }

      if(thread->Priority > maxPriority) {
         maxPriority = thread->Priority;
         maxPriorityThread = thread;
      }
   }

   if(maxPriorityThread != Scheduler_currentThread) {
      #ifdef SATORU_DEBUG
      CheckStackUsage();
      #endif
      CheckStackIntegrity();
      Scheduler_pendingSwitchThread = maxPriorityThread;
      Context_Switch();
   }
}

}

}
