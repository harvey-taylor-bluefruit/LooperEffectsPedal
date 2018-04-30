// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Gate.h"
#include "SatoruConfig.h"
#include "SatoruPort.h"
#include "CriticalSection.h"
#include "Satoru.h"
#include "Resources.h"
#include "Tick.h"
#include "../internals/Thread.h"

#include <cstddef>

namespace satoru
{

Gate::Gate() {}

Gate::~Gate()
{
   Open();
}

void Gate::Wait()
{
   if (!core::IsRunning()) {
      return;
   }

   { CriticalSection section;
      Scheduler_currentThread->BlockingPrimitive = this;
      Scheduler_currentThread->Status = ThreadStatus_Suspended;
   }

   core::SelectNextThreadToRun();
}

bool Gate::Wait(size_t timeoutMs)
{
   if (!core::IsRunning() || timeoutMs == 0) {
      return false;
   }

   { CriticalSection section;
      Scheduler_currentThread->BlockingPrimitive = this;
      Scheduler_currentThread->Status = ThreadStatus_Suspended;
      Scheduler_currentThread->AbsoluteTicksToActive = resources.tickCounter 
                                                       + port::MsToTicks(timeoutMs);
      Scheduler_currentThread->IsAsleep = true;
      WakeUpCycle();
   }
   core::SelectNextThreadToRun();

   // We get here when scheduled back, and check if we timed out.
   { CriticalSection section;
      if (Scheduler_currentThread->BlockingPrimitive == this) {
         // We are still blocking on the gate, so we were re-scheduled
         // via the tick handler (timeout).
         Scheduler_currentThread->BlockingPrimitive = NULL;
         return false;
      }
   }

   return true;
}

void Gate::Open()
{
   if (!core::IsRunning()) {
      return;
   }

   bool mustYield = false;
   { CriticalSection section;
      for (size_t i = 1U; i < config::MaxNumberOfThreads; i++) {
         Thread *thread = &resources.threadPool[i];
         if((thread->Status == ThreadStatus_Suspended) && (thread->BlockingPrimitive == this)) {
            thread->Status = ThreadStatus_Active;
            thread->BlockingPrimitive = NULL;
            mustYield = true;
         }
      }
   }

   if (mustYield) {
      core::SelectNextThreadToRun();
   }
}

}
