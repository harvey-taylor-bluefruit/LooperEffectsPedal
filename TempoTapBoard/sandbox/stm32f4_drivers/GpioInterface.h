#ifndef GPIO_INTERFACE_H
#define GPIO_INTERFACE_H

class GpioInterface
{
public:
   struct PinState {
      enum Value {
         Low = 0,
         High = 1
      };
   };

   virtual ~GpioInterface() {};

   virtual void ConfigureAsInput() = 0;
   virtual void ConfigureAsOutput(PinState::Value initialState) = 0;

   virtual void SetOutputHigh() = 0;
   virtual void SetOutputLow() = 0;

   virtual PinState::Value GetPinState() = 0;
};

#endif
