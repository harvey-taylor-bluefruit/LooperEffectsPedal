#include "Cache.h"
#include "stm32f7xx.h"

namespace stm32_drivers {

namespace Cache {

void Enable()
{
   SCB_EnableICache();
   SCB_EnableDCache();
}

}

}
