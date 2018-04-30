#include "Gpio.h"
#include "Clocks.h"

namespace stm32_drivers {

Gpio::Gpio(const ::stm32_drivers::GpioTypeDefinitions::IoPin &ioPin) :
   mPinIndex(ioPin.PinIndex),
   mPinMask(1U << ioPin.PinIndex),
   mPort(ioPin.Port->Registers)
{
   if(!Clocks::IsPeripheralClockEnabled(ioPin.Port->PeripheralClockEnable, ioPin.Port->PeripheralIndex)) {
      Clocks::EnablePeripheralClock(ioPin.Port->PeripheralClockEnable, ioPin.Port->PeripheralIndex);
   }
}

void Gpio::ConfigureAsInput()
{
   SetGpioMode(&mPort->MODER, GpioMode::Input, mPinIndex);
}

void Gpio::ConfigureAsOutput(GpioInterface::PinState::Value initialState)
{
   if (initialState == GpioInterface::PinState::High) {
      SetOutputHigh();
   } else {
      SetOutputLow();
   }
   SetGpioMode(&mPort->MODER, GpioMode::Output, mPinIndex);
}

void Gpio::SetOutputHigh()
{
   mPort->BSRR = 1U << mPinIndex;
}

void Gpio::SetOutputLow()
{
   static const uint8_t ResetIndex = 16U;
   mPort->BSRR = 1U << (ResetIndex + mPinIndex);
}

GpioInterface::PinState::Value Gpio::GetPinState()
{
   return ((mPort->IDR & (1U << mPinIndex)) != 0U) ? PinState::High : PinState::Low;
}

void Gpio::SetGpioMode(volatile uint32_t *modeRegister, uint8_t mode, uint8_t pinIndex)
{
   const uint8_t ModePosition = static_cast<uint8_t>(pinIndex * 2U);
   *modeRegister &= ~(0x3 << ModePosition);
   *modeRegister |= (mode & 0x3) << ModePosition;
}

void Gpio::SetAlternateFunction(volatile uint32_t *alternateFunctionRegisters, uint8_t alternateFunction, uint8_t pinIndex)
{
   const uint8_t AlternateFunctionRegisterIndex = pinIndex / 8U;
   const uint8_t AlternateFunctionPosition = static_cast<uint8_t>((pinIndex % 8) * 4);
   alternateFunctionRegisters[AlternateFunctionRegisterIndex] &= ~(0x0F << AlternateFunctionPosition);
   alternateFunctionRegisters[AlternateFunctionRegisterIndex] |= (alternateFunction & 0xF) << AlternateFunctionPosition;
}

void Gpio::SetSpeed(GpioSpeed::Value speed)
{
   const uint8_t SpeedPosition = static_cast<uint8_t>(mPinIndex * 2U);
   mPort->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << SpeedPosition);
   mPort->OSPEEDR |= (speed << SpeedPosition);
}

void Gpio::SetPuPd(GpioPuPd::Value pupd)
{
   const uint8_t PuPdPosition = static_cast<uint8_t>(mPinIndex * 2U);
   mPort->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << PuPdPosition);
   mPort->PUPDR |= (pupd << PuPdPosition);
}

void Gpio::SetOutputType(GpioOutputType::Value outputType)
{
   const uint8_t OutputTypePosition = static_cast<uint8_t>(mPinIndex);
   mPort->OTYPER &= ~(GPIO_OTYPER_OT_0 << OutputTypePosition);
   mPort->OTYPER |= (outputType << OutputTypePosition);
}

}
