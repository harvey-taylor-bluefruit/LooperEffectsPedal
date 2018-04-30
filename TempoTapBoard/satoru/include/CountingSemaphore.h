// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef COUNTING_SEMAPHORE_H
#define COUNTING_SEMAPHORE_H

#include <stdint.h>
#include <stddef.h>

namespace satoru 
{

class CountingSemaphore 
{
public:
   CountingSemaphore(size_t initialCount = 0);

   void Take();
   void Give();
   bool Take(size_t timeoutMs); // returns false on timeout.
   size_t Count() const;

private:
   size_t mCount;
   CountingSemaphore(const CountingSemaphore&);
   const CountingSemaphore& operator=(const CountingSemaphore&);
};

}

#endif

