#ifndef GPIO_H
#define GPIO_H

#include "GpioInterface.h"
#include "GpioTypeDefinitions.h"

namespace stm32_drivers {

using namespace GpioTypeDefinitions;

class Gpio : public GpioInterface
{
public:
   explicit Gpio(const IoPin &ioPin);
   void ConfigureAsInput();
   void ConfigureAsOutput(GpioInterface::PinState::Value initialState);

   void SetOutputHigh();
   void SetOutputLow();

   PinState::Value GetPinState();

   void SetSpeed(GpioSpeed::Value speed);
   void SetPuPd(GpioPuPd::Value pupd);
   void SetOutputType(GpioOutputType::Value outputType);

   static void SetAlternateFunction(volatile uint32_t *afRegisters,
                                    uint8_t alternateFunction,
                                    uint8_t pinIndex);
   static void SetGpioMode(volatile uint32_t *modeRegister,
                           uint8_t mode,
                           uint8_t pinIndex);

private:
   const uint8_t mPinIndex;
   const uint8_t mPinMask;
   GPIO_TypeDef *mPort;
};

}

#endif
