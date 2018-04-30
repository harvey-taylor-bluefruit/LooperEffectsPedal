#include "panku.h"
#include "Clocks.h"
#include "IOPinMap.h"
#include "Uart.h"

PANKU_LIST
(
   STANDALONE(::stm32_drivers::Clocks),
   DEPENDENCY(::sandbox::IOPinMap, ::stm32_drivers::Clocks),
   DEPENDENCY(::stm32_drivers::Uart, ::sandbox::IOPinMap)
);

extern panku devices;
