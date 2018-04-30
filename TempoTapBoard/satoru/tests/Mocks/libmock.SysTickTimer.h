#pragma once

#include <stdint.h>
#include <stddef.h>

class LibMockSysTickTimer
{
public:
   static void Initialise();

   static size_t mDelayUsCalls;
   static size_t mLastDelay_us;
};
