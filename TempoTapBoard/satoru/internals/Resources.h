// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SCHEDULER_RESOURCES_H
#define SCHEDULER_RESOURCES_H

#include "SatoruConfig.h"
#include "TimerEntry.h"
#include "Thread.h"

namespace satoru 
{

struct Resources 
{
   Resources();

   // Constants
   static const uint8_t KernelThreadPriority = 0U;
   static const uint8_t ReservedPriorityRange = 10u;
   static const uint8_t LowestPriority = KernelThreadPriority + 1U;
   static const uint8_t HighestPriority = UINT8_MAX - ReservedPriorityRange;
   static const uint8_t TimerThreadPriority = HighestPriority + 1U;
   static const uint8_t KernelThreadIndex = 0U;
   static const uint8_t TimerThreadIndex = 1U;
   static const size_t StackBottomCanary = 0xCA9A41D1;
   static const uint8_t StackFillValue = 0xA5;

   // State
   bool isRunning;

   // Threads
   Thread threadPool[config::MaxNumberOfThreads];
   uint8_t createdThreadCount;

   // Timing
   size_t tickCounter;
   TimerEntry* firstActiveTimer;
   TimerEntry* firstPendingTimer;
   CountingSemaphore timerThreadSemaphore;
   TimerEntry timerList[config::MaximumActiveTimers];

   // Called by static constructor.
   void Reset();
};

extern struct Resources resources;

extern "C" Thread *Scheduler_KernelThread;
extern "C" Thread *Scheduler_TimerThread;
extern "C" Thread *Scheduler_currentThread;
extern "C" Thread *Scheduler_pendingSwitchThread;

}

#endif
