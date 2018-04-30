#ifndef BYTE_RECEIVE_INTERRUPT_CALLBACK_INTERFACE_H
#define BYTE_RECEIVE_INTERRUPT_CALLBACK_INTERFACE_H

#include <stdint.h>

class ByteReceiveInterruptCallbackInterface
{
public:
   virtual ~ByteReceiveInterruptCallbackInterface() {}
   virtual void OnReceiveInterrupt(uint8_t byte) = 0;
};

#endif
