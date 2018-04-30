// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Satoru.h"

#include "SatoruAssert.h"
#include "SatoruConfig.h"
#include "SatoruDebug.h"
#include "SatoruPort.h"

#include "Context.h"
#include "Critical.h"
#include "CriticalSection.h"
#include "CountingSemaphore.h"
#include "StackChecks.h"
#include "TimerEntry.h"
#include "Resources.h"
#include "Tick.h"
#include "DataAlign.h"

#include <cstring>
#include <cstdio>

namespace satoru
{

const uint8_t core::LowestPriority = Resources::LowestPriority;
const uint8_t core::HighestPriority = Resources::HighestPriority;

DATA_ALIGN_8(uint8_t kernelThreadStack[config::KernelThreadStackSize]);
DATA_ALIGN_8(uint8_t timerThreadStack[config::TimerThreadStackSize]);

#ifdef __arm__
#define NAKED_FUNCTION  __attribute((naked))
#else
#define NAKED_FUNCTION
#endif

NAKED_FUNCTION static void KernelThreadMain()
{
#ifdef SATORU_DEBUG
   Critical_DisallowInterrupts();
   size_t handlerStackAddress = port::GetHandlerStackPointer();
   handlerStackAddress -= config::HandlerStackMaxSafeUsage;
   memset(reinterpret_cast<void *>(handlerStackAddress), Resources::StackFillValue, config::HandlerStackMaxSafeUsage);
   Critical_AllowInterrupts();
#endif

   port::StartTick();
   core::SelectNextThreadToRun();
   for(;;) {
   }
}

extern void ServiceTimerList();
NAKED_FUNCTION static void TimerThreadMain()
{
   for(;;) {
      ServiceTimerList();
   }
}

NAKED_FUNCTION static void ThreadShutdown()
{
   core::ThreadTerminate();
   for(;;) {
   }
}

bool core::IsRunning()
{
   return resources.isRunning;
}

void core::CreateThread(void *stackBuffer,
                        size_t stackSize,
                        void (*entryPoint) (),
                        uint8_t priority)
{
   core::CreateThread("[unnamed]", stackBuffer, stackSize, entryPoint, priority);
}

void core::CreateThread(const char *name,
                        void *stackBuffer,
                        size_t stackSize,
                        void (*entryPoint) (),
                        uint8_t priority)
{
   CriticalSection section;

   // Only the timer thread can be over the highest user priority.
   SatoruAssert((entryPoint == TimerThreadMain) || (priority <= HighestPriority));
   SatoruAssert(resources.createdThreadCount < config::MaxNumberOfThreads);

   // Find next available slot
   uint8_t nextAvailableSlot = 0U;
   for (size_t i = 0; i < config::MaxNumberOfThreads; i++) {
      if (resources.threadPool[i].Status == ThreadStatus_Free) {
         nextAvailableSlot = static_cast<uint8_t>(i);
         break;
      }
   }

   // Sanity check
   SatoruAssert(nextAvailableSlot < config::MaxNumberOfThreads);

   // Good to go
   resources.threadPool[nextAvailableSlot].Name = name;
   resources.threadPool[nextAvailableSlot].Status = ThreadStatus_Active;
   resources.threadPool[nextAvailableSlot].IsAsleep = false;
   resources.threadPool[nextAvailableSlot].StackBottom = stackBuffer;
   resources.threadPool[nextAvailableSlot].SavedStackPointer = Context_InitialiseStackFrames(stackBuffer, stackSize, entryPoint, ThreadShutdown);
   resources.threadPool[nextAvailableSlot].EntryPoint = entryPoint;
   resources.threadPool[nextAvailableSlot].Priority = priority;

   // Fill stack with known value to detect max stack usage
   memset(stackBuffer, Resources::StackFillValue,
          (static_cast<uint8_t*>(resources.threadPool[nextAvailableSlot].SavedStackPointer) -
           static_cast<uint8_t*>(resources.threadPool[nextAvailableSlot].StackBottom)));

   uint32_t *stackCanaryPointer = static_cast<uint32_t *>(stackBuffer);
   *stackCanaryPointer = Resources::StackBottomCanary;

   resources.createdThreadCount++;
}

// Called at resources initialisation.
void CreateInitialThreads()
{
   core::CreateThread("Kernel",
                kernelThreadStack,
                config::KernelThreadStackSize,
                KernelThreadMain,
                Resources::KernelThreadPriority);
   core::CreateThread("Timer",
                timerThreadStack,
                config::TimerThreadStackSize,
                TimerThreadMain,
                Resources::TimerThreadPriority);
}

void core::Start()
{
   for (size_t i = 1; i < config::MaxNumberOfThreads; i++) {
      if ((resources.threadPool[i].Status == ThreadStatus_Suspended) ||
          (resources.threadPool[i].Status == ThreadStatus_Active)) {
         resources.isRunning = true;
         break;
      }
   }

   SatoruAssert(resources.isRunning);
   port::Initialise();

   // This is never going to return - we are now going to throw
   // ourselves into the kernel context (stack will be the kernel
   // stack)
   Context_FirstSwitchToKernelThread();
}

void core::ThreadSleepMilliseconds(size_t milliseconds)
{
   ThreadSleepTicks(port::MsToTicks(milliseconds));
}

size_t core::Ticks()
{
   return resources.tickCounter;
}

}
