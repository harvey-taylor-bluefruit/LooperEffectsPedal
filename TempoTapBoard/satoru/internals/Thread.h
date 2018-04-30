// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
namespace satoru
{
#endif

typedef enum
{
   ThreadStatus_Free,
   ThreadStatus_Suspended,
   ThreadStatus_Active
} ThreadStatus;

typedef uint8_t ThreadHandle;

typedef struct
{
   void *SavedStackPointer;     // needs to be at the top of the struct
   void *StackBottom;           // descending stack
   void (*EntryPoint) (void);
   ThreadStatus Status;
   size_t AbsoluteTicksToActive;
   bool IsAsleep;
   void *BlockingPrimitive;
   uint8_t Priority;
   const char *Name;
#ifdef SATORU_THREAD_WATCHDOG
   size_t WatchdogPeriodTicks;
   size_t AbsoluteLastWatchdogKick;
#endif
} Thread;

#ifdef __cplusplus
namespace core
{
#endif
   void SelectNextThreadToRun();
   void ThreadSleepTicks(size_t ticks);
   void ThreadTerminate();
#ifdef __cplusplus
}

}
#endif 

#endif
