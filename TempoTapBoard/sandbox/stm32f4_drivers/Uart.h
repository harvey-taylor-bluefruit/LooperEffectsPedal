// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef UART_H
#define UART_H

#include "UartInterface.h"
#include "stdint.h"
#include "stdlib.h"

namespace stm32_drivers {

class Uart : public UartInterface
{
public:
   enum class Instance {
      Usart3,
      Usart6,
      Count
   };

   Uart(Instance instance, uint32_t baudRate);

   void Enable();
   void SendStringBlocking(const char *message);
   void SetReceiveCallback(ByteReceiveInterruptCallbackInterface *callback);

private:
   Instance mInstance;
   uint32_t mBaudRate;
   size_t mIndex;
   struct UartPeripheral *mPeripheral;

   void SetBaudRate() const;
   void SendByteBlocking(uint8_t byte) const;

   ByteReceiveInterruptCallbackInterface *mReceiveCallback;
};

}

#endif
