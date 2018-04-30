// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Resources.h"
#include "SatoruPort.h"

#include <string.h>
#include <stddef.h>

namespace satoru 
{

Thread *Scheduler_KernelThread;
Thread *Scheduler_TimerThread;
Thread *Scheduler_currentThread;
Thread *Scheduler_pendingSwitchThread;

struct Resources resources;
extern void CreateInitialThreads();

void Resources::Reset()
{
   isRunning = false;
   createdThreadCount = 0u;
   tickCounter = 0u;

   firstActiveTimer = NULL;
   firstPendingTimer = NULL;

   Scheduler_KernelThread = &threadPool[KernelThreadIndex];
   Scheduler_TimerThread = &threadPool[TimerThreadIndex];
   Scheduler_currentThread = Scheduler_KernelThread;
   Scheduler_pendingSwitchThread = NULL;

   memset(threadPool, 0, sizeof threadPool);

   for (size_t i = 0; i < config::MaxNumberOfThreads; i++) {
      threadPool[i].Status = ThreadStatus_Free;
      threadPool[i].Priority = LowestPriority;
   }

   CreateInitialThreads();
}

Resources::Resources()
{
   Reset();
}

}
