// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "RecursiveMutex.h"
#include "SatoruConfig.h"
#include "CriticalSection.h"
#include "Satoru.h"
#include "Resources.h"
#include "Thread.h"

#include <cstddef>

namespace satoru
{

extern "C" Thread *Scheduler_currentThread;

void RecursiveMutex::Lock()
{
   if (!core::IsRunning()) {
      return;
   }

   bool mustYield = false;

   { CriticalSection section;
      if (IsLocked() && (mOwner != Scheduler_currentThread)) {
         Scheduler_currentThread->BlockingPrimitive = this;
         Scheduler_currentThread->Status = ThreadStatus_Suspended;
         mustYield = true;
      } else {
         mOwner = Scheduler_currentThread;
      }
   }
   
   if (mustYield) {
      core::SelectNextThreadToRun();
   }
}

void RecursiveMutex::Unlock()
{
   if (!core::IsRunning()) {
      return;
   }

   bool mustYield = false;
   { CriticalSection section;
      if (mOwner != Scheduler_currentThread) {
         return;
      }

      for (size_t i = 1U; i < config::MaxNumberOfThreads; i++) {
         Thread *thread = &resources.threadPool[i];
         if((thread->Status == ThreadStatus_Suspended) && (thread->BlockingPrimitive == this)) {
            thread->Status = ThreadStatus_Active;
            thread->BlockingPrimitive = NULL;
            mustYield = true;
         }
      }

      mOwner = NULL;
   }

   if (mustYield) {
      core::SelectNextThreadToRun();
   }
}

bool RecursiveMutex::IsLocked() const
{
   return mOwner != NULL;
}

RecursiveMutex::RecursiveMutex():
   mOwner(NULL) 
{}

RecursiveMutex::~RecursiveMutex() 
{
   Lock(); // Can only be destroyed by the owning thread.
}

}
