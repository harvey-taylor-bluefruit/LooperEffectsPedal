// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef RECURSIVE_MUTEX_H
#define RECURSIVE_MUTEX_H

#include "Thread.h"

namespace satoru
{

class RecursiveMutex 
{
public:
   void Lock();
   bool IsLocked() const;
   void Unlock();

   RecursiveMutex();
   ~RecursiveMutex();

private:
   Thread* mOwner;
   RecursiveMutex(const RecursiveMutex&);
   const RecursiveMutex& operator=(const RecursiveMutex&);
};

}

#endif

