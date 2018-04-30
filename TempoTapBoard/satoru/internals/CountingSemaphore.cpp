// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "CountingSemaphore.h"
#include "SatoruAssert.h"
#include "SatoruConfig.h"
#include "SatoruPort.h"
#include "../internals/Resources.h"
#include "CriticalSection.h"
#include "Satoru.h"
#include "../internals/Thread.h"
#include "../internals/Tick.h"

#include <stddef.h>

namespace satoru 
{

CountingSemaphore::CountingSemaphore(size_t count):
   mCount(count)
{}

void CountingSemaphore::Take()
{
   if (!core::IsRunning()) {
      return;
   }
   bool mustYield = false;

   { CriticalSection section;
      if (mCount == 0) {
         Scheduler_currentThread->BlockingPrimitive = this;
         Scheduler_currentThread->Status = ThreadStatus_Suspended;
         mustYield = true;
      } else {
         mCount--;
      }
   }

   if (mustYield) {
      core::SelectNextThreadToRun();
   }
}

bool CountingSemaphore::Take(size_t timeoutMs)
{
   if (!core::IsRunning()) {
      return false;
   }
   bool mustYield = false;

   { CriticalSection section;
      if (mCount == 0) {
         if (timeoutMs == 0) {
            return false;
         }
         Scheduler_currentThread->BlockingPrimitive = this;
         Scheduler_currentThread->Status = ThreadStatus_Suspended;
         Scheduler_currentThread->IsAsleep = true;
         Scheduler_currentThread->AbsoluteTicksToActive = resources.tickCounter 
                                                          + port::MsToTicks(timeoutMs);
         WakeUpCycle();
         mustYield = true;
      } else {
         mCount--;
      }
   }
   if (mustYield) {
      core::SelectNextThreadToRun();

      { CriticalSection section;
         if (Scheduler_currentThread->BlockingPrimitive == this) {
            // We are still blocking on the semaphore, so we were re-scheduled
            // via the tick handler (timeout).
            Scheduler_currentThread->BlockingPrimitive = NULL;
            return false;
         }
      }
   }

   return true;
}

size_t CountingSemaphore::Count() const
{
   return mCount;
}

void CountingSemaphore::Give()
{
   if (!core::IsRunning()) {
      return;
   }

   bool mustYield = false;
   { CriticalSection section;
      mCount++; 
      for (size_t i = 1U; i < config::MaxNumberOfThreads; i++) {
         Thread *thread = &resources.threadPool[i];
         if((thread->Status == ThreadStatus_Suspended) && (thread->BlockingPrimitive == this)) {
            thread->Status = ThreadStatus_Active;
            thread->IsAsleep = false;
            thread->BlockingPrimitive = NULL;
            mCount--;
            mustYield = true;
            break; // Only one unblock per give.
         }
      }
   }

   if (mustYield) {
      core::SelectNextThreadToRun();
   }

}


}
