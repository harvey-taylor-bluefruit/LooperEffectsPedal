#include "SysTickTimer.h"
#include "libmock.SysTickTimer.h"

size_t LibMockSysTickTimer::mLastDelay_us;
size_t LibMockSysTickTimer::mDelayUsCalls;

namespace stm32_drivers {

void SysTickTimer::BlockingDelayUs(size_t delayUs) {
   LibMockSysTickTimer::mLastDelay_us = delayUs;
   LibMockSysTickTimer::mDelayUsCalls++;
}

void SysTickTimer::BlockingDelayMs(size_t delayMs) { (void)delayMs; }

} 

void LibMockSysTickTimer::Initialise()
{
   mLastDelay_us = 0U;
   mDelayUsCalls = 0U;
}

namespace satoru { namespace port {
   void StartTick() {}
}}

