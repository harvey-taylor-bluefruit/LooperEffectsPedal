// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Spi.h"
#include "Clocks.h"
#include "SysTickTimer.h"
#include "SatoruAssert.h"
#include "Wait.h"

namespace stm32_drivers
{

 namespace {
   volatile uint32_t *SPI1PeripheralClockEnable = &RCC->APB2ENR;
   volatile uint32_t *SPI3PeripheralClockEnable = &RCC->APB1ENR;
   volatile uint32_t *SPI1PeripheralReset = &RCC->APB2RSTR;
   volatile uint32_t *SPI3PeripheralReset = &RCC->APB1RSTR;
   volatile uint32_t *SPI4PeripheralClockEnable = &RCC->APB2ENR;
   volatile uint32_t *SPI4PeripheralReset = &RCC->APB2RSTR;
   uint8_t SPI1PeripheralIndex = 12U;
   uint8_t SPI3PeripheralIndex = 15U;
   uint8_t SPI4PeripheralIndex = 13U;
}

Spi::Spi(Instance instance,
         GpioInterface& chipSelectPin,
         SpiMode mode,
         bool txOnly):
   mChipSelectPin(chipSelectPin),
   mMode(mode),
   mTxOnly(txOnly)
{
   mChipSelectPin.ConfigureAsOutput(GpioInterface::PinState::High);

   switch (instance)
   {
   case Instance::Spi1:
      mSpiRegisters = SPI1;
      Clocks::ResetPeripheral(SPI1PeripheralReset,
                              SPI1PeripheralIndex);
      Clocks::EnablePeripheralClock(SPI1PeripheralClockEnable,
                                    SPI1PeripheralIndex);
      break;
   case Instance::Spi3:
      mSpiRegisters = SPI3;
      Clocks::ResetPeripheral(SPI3PeripheralReset,
                              SPI3PeripheralIndex);
      Clocks::EnablePeripheralClock(SPI3PeripheralClockEnable,
                                    SPI3PeripheralIndex);
      break;
   case Instance::Spi4:
      mSpiRegisters = SPI4;
      Clocks::ResetPeripheral(SPI4PeripheralReset,
                              SPI4PeripheralIndex);
      Clocks::EnablePeripheralClock(SPI4PeripheralClockEnable,
                                    SPI4PeripheralIndex);
      break;
   default:
      FATAL("Unsupported instance");
      break;
   }

  InitializeMasterMode();
}

void Spi::InitializeMasterMode()
{
   mSpiRegisters->CR1 &= ~SPI_CR1_BR;
   mSpiRegisters->CR1 |= (BaudRateDividerLevel << 3);

   switch (mMode) {
      case SpiMode::Zero:
         // SPIMode Zero is when CPOL=0, CPHA = 0
         mSpiRegisters->CR1 &= ~SPI_CR1_CPOL;
         mSpiRegisters->CR1 &= ~SPI_CR1_CPHA;
         break;
      case SpiMode::One:
         // SPIMode One is when CPOL=0, CPHA = 1
         FATAL("SpiMode 1 not implemented yet");
         break;
      case SpiMode::Two:
         // SPIMode Two is when CPOL=1, CPHA = 0
         FATAL("SpiMode 2 not implemented yet");
         break;
      case SpiMode::Three:
         // SPIMode Three is when CPOL=1, CPHA = 1
         mSpiRegisters->CR1 |= SPI_CR1_CPOL;
         mSpiRegisters->CR1 |= SPI_CR1_CPHA;
         break;
   };

   // Software Slave Management
   mSpiRegisters->CR1 |= SPI_CR1_SSM;

   if (mTxOnly) {
      mSpiRegisters->CR1 |= SPI_CR1_BIDIMODE;
      mSpiRegisters->CR1 |= SPI_CR1_BIDIOE;
      mSpiRegisters->CR1 &= ~SPI_CR1_RXONLY;
   }

   // Master mode and general enable
   mSpiRegisters->CR2 |= SPI_CR2_SSOE | SPI_CR2_FRXTH; // 8 bit threshold for FIFO, so that single bytes generate RXNE events (no data packing)
   mSpiRegisters->CR1 |= SPI_CR1_MSTR;
   mSpiRegisters->CR1 |= SPI_CR1_SPE;
}

void Spi::Enable()
{
   mChipSelectPin.SetOutputLow();
}

bool Spi::Disable()
{
   if (!WaitForTxe()) {
      return false;
   }

   if (!WaitUntilNotBusy()) {
      return false;
   }

   mChipSelectPin.SetOutputHigh();
   return true;
}

bool Spi::TransmitAndReceive(uint8_t tx, uint8_t &rx)
{
   SatoruAssert(!mTxOnly);

   const uint32_t ChecksBeforeUsWait = 100U;
   const uint32_t usToWait = 5U;
   bool written = false;
   bool read = false;

   for (uint32_t i = 0; i != usToWait; i++) {
      for (uint32_t check = 0U; check < ChecksBeforeUsWait; check++) {
         const bool txe = (mSpiRegisters->SR & SPI_SR_TXE);
         if (txe) {
            // Write byte - forcing "store byte" instruction to disable "data packing" feature on STM32F7
            * (reinterpret_cast<volatile uint8_t*>(&mSpiRegisters->DR)) = tx;
            written = true;
            break;
         }
      }
      if (written) {
         break;
      }
      WaitMs(1U);
   }

   if (!written) {
      return false;
   }

   for (uint32_t i = 0; i != usToWait; i++) {
      for (uint32_t check = 0U; check < ChecksBeforeUsWait; check++) {
         const bool rxne = (mSpiRegisters->SR & SPI_SR_RXNE);
         if (rxne) {
            // Read byte
            rx = mSpiRegisters->DR;
            read = true;
            break;
         }
      }
      if (read) {
         break;
      }
      WaitMs(1U);
   }

   if (!read) {
      return false;
   }

   Spi::WaitUntilNotBusy();
   return true;
}

bool Spi::Transmit(uint8_t tx)
{
   const uint32_t ChecksBeforeUsWait = 100U;
   const uint32_t usToWait = 5U;
   bool written = false;

   for (uint32_t i = 0; i != usToWait; i++) {
      for (uint32_t check = 0U; check < ChecksBeforeUsWait; check++) {
         const bool txe = (mSpiRegisters->SR & SPI_SR_TXE);
         if (txe) {
            // Write byte
            * (reinterpret_cast<volatile uint8_t*>(&mSpiRegisters->DR)) = tx;
            written = true;
            break;
         }
      }
      if (written) {
         break;
      }
      WaitMs(1U);
   }

   if (!written) {
      return false;
   }

   Spi::WaitUntilNotBusy();
   return true;
}

bool Spi::WaitUntilNotBusy() const
{
   const uint32_t ChecksBeforeUsWait = 100U;
   uint32_t msToWait = 5U;
   for (uint32_t i = 0; i != msToWait; i++) {
      for (uint32_t check = 0U; check < ChecksBeforeUsWait; check++) {
         bool busy = (mSpiRegisters->SR & SPI_SR_BSY);
         if (!busy) {
            return true;
         }
      }
      WaitMs(1U);
   }
   return false;  // timeout
}

bool Spi::WaitForTxe() const
{
   const uint32_t ChecksBeforeUsWait = 100U;
   const uint32_t msToWait = 5U;

   for (uint32_t i = 0; i != msToWait; i++) {
      for (uint32_t check = 0U; check < ChecksBeforeUsWait; check++) {
         const bool txe = (mSpiRegisters->SR & SPI_SR_TXE);
         if (txe) {
            return true;
         }
      }
      WaitMs(1U);
   }
   return false;  // timeout
}

}
