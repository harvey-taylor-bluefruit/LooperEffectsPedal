#pragma once

#include "UartInterface.h"
#include <cstring>

class NullMockUart: public UartInterface
{
public:
   virtual void Enable() override {}
   virtual void SendStringBlocking(const char *) override  {}
   virtual void SetReceiveCallback(ByteReceiveInterruptCallbackInterface*) override {}
};

