#ifndef UART_H
#define UART_H

#include "UartInterface.h"

namespace stm32_drivers {

class Uart : public UartInterface
{
public:
   struct UartRegisters;

   enum class Instance {
      Usart3,
      Usart6
   };

   Uart(Instance instance, uint32_t baudRate);

   void Enable();
   void SendStringBlocking(const char *message) const;
   void SendByteBlocking(uint8_t byte) const;

   void SetReceiveCallback(ByteReceiveInterruptCallbackInterface *callback);
private:
   const UartRegisters *mUartRegisters;
   uint32_t mBaudRate;

   void SetBaudRate() const;
};

}

#endif
