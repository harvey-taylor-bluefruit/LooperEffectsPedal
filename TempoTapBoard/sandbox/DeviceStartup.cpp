#include "IOPinMap.h"
#include "DeviceStartup.h"
#include "TerminalDebug.h"

using namespace ::sandbox;
using namespace ::stm32_drivers;

template<>
Uart& ConstructAndInitialise<Uart&>() {
   static const uint32_t TerminalBaudRate = 115200U;
   static Uart uart(Uart::Instance::Usart3, TerminalBaudRate);
   uart.Enable();
   ::sandbox::terminalUart = &uart;
   return uart;
}

template<>
IOPinMap& ConstructAndInitialise<IOPinMap&>() {
   static IOPinMap map;
   return map;
}

template<>
Clocks& ConstructAndInitialise<Clocks&>() {
   static Clocks clocks;
   return clocks;
}
