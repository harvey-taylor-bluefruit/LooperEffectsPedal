#include "Wait.h"

#include "stm32f7xx.h"
#include "SysTickTimer.h"
#include "Satoru.h"
#include <stdint.h>

namespace stm32_drivers {

void WaitMs(uint32_t ms)
{
   if (!satoru::core::IsRunning()) {
      SysTickTimer::BlockingDelayMs(ms);
      return;
   }

   satoru::core::ThreadSleepMilliseconds(ms);
}

}
