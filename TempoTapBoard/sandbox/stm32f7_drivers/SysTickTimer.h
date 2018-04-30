#ifndef SYSTICK_TIMER_H
#define SYSTICK_TIMER_H

#include <stddef.h>
#include <stdint.h>

namespace stm32_drivers {

namespace SysTickTimer
{
   void BlockingDelayUs(size_t delayUs);
   void BlockingDelayMs(size_t delayMs);
}

}

#endif
