// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SATORU_H
#define SATORU_H

#include <stdint.h>
#include <stddef.h>

namespace satoru 
{

namespace core
{
   extern const uint8_t LowestPriority;
   extern const uint8_t HighestPriority;

   void CreateThread(const char *name,
                     void *stackBuffer,
                     size_t stackSize,
                     void (*EntryPoint) (),
                     uint8_t priority = LowestPriority);

   void CreateThread(void *stackBuffer,
                     size_t stackSize,
                     void (*EntryPoint) (),
                     uint8_t priority = LowestPriority);
   void Start();
   bool IsRunning();

   void ThreadSleepMilliseconds(size_t milliseconds);

   size_t Ticks();
}

}

#endif
