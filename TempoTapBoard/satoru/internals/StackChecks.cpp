// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "StackChecks.h"
#include "SatoruDebug.h"
#include "SatoruPort.h"
#include "Resources.h"
#include "Thread.h"
#include "SatoruAssert.h"

namespace satoru 
{

#ifdef SATORU_DEBUG
void CheckStackUsage()
{
   if(Scheduler_currentThread == Scheduler_KernelThread) {
      return;
   }
   uint8_t* lowestStackPointer = reinterpret_cast<uint8_t*>(Scheduler_currentThread->StackBottom) + sizeof(Resources::StackBottomCanary);
   while (lowestStackPointer != Scheduler_currentThread->SavedStackPointer) { // worst case for stack pointer
      if (*lowestStackPointer != Resources::StackFillValue) {
         break;
      }
      lowestStackPointer++;
   }

   if (lowestStackPointer < (static_cast<uint8_t*>(Scheduler_currentThread->StackBottom) + debug::StackSafetyLimitBytes)) {
      debug::Print("Warning: Stack usage safety level exceeded, please increase stack size for thread! name: \"");
      debug::Print(Scheduler_currentThread->Name);
      debug::Print("\"\r\n");
   }
}
#endif

void CheckStackIntegrity()
{
   // First check: stack bottom "canary" has not been overwritten
   const uint32_t *stackCanaryPointer = static_cast<uint32_t *>(Scheduler_currentThread->StackBottom);
   SatoruAssert(*stackCanaryPointer == Resources::StackBottomCanary);

#if !defined(TESTING) // this check only works on the target
   // Second check: process stack pointer is currently above the stack bottom
   const uint32_t ProcessStackPointer = port::GetThreadStackPointer();
   SatoruAssert(reinterpret_cast<uint8_t *>(ProcessStackPointer) > static_cast<uint8_t *>(Scheduler_currentThread->StackBottom));

#ifdef SATORU_DEBUG
   // Third check: handler stack usage is less than limit
   const uint32_t HandlerStackPointer = port::GetHandlerStackPointer();
   uint8_t* fillValuePointer = reinterpret_cast<uint8_t *>(HandlerStackPointer);
   for (size_t i = 0U; i < config::HandlerStackMaxSafeUsage; i++) {
      if (*fillValuePointer == Resources::StackFillValue) {
         break;
      }
      fillValuePointer--;
   }
   if(*fillValuePointer != Resources::StackFillValue) {
      FATAL("Handler Stack Usage exceeded safe level:\r\n" \
            "1. Check stack usage in interrupt context\r\n" \
            "2. Increase satoru::config::HandlerStackMaxSafeUsage if necessary\r\n" \
            "3. Make sure stack size defined in linker script is greater than HandlerStackMaxSafeUsage, increase if needed\r\n");
   }
#endif
#endif

}

}
