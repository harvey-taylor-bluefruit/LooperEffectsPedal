// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "CriticalSection.h"
#include "Critical.h"

#include <stdint.h>
#include <stddef.h>

namespace satoru 
{

namespace 
{
   size_t aNestingCount = 0;
}

CriticalSection::CriticalSection()
{
   Critical_DisallowInterrupts();
   aNestingCount++;
}

CriticalSection::~CriticalSection()
{
   aNestingCount--; // Safe, since it is unreachable without having called the default constructor.
   if (aNestingCount == 0) {
      Critical_AllowInterrupts();
   }
}

}
