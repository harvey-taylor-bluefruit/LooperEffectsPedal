#include "Spi.h"

#include "SatoruAssert.h"
#include "stm32f4xx.h"
#include "Clocks.h"
#include "InterruptPriorities.h"

#include "Wait.h"

#include <cstddef>

namespace sandbox { namespace STM32Drivers {

namespace {
   Spi* aSpiInstance[Spi::NumberOfInstances] = {NULL, NULL, NULL};
   volatile uint32_t *SPI1PeripheralClockEnable = &RCC->APB2ENR;
   volatile uint32_t *SPI1PeripheralReset = &RCC->APB2RSTR;
   volatile uint32_t *SPI2PeripheralClockEnable = &RCC->APB1ENR;
   volatile uint32_t *SPI2PeripheralReset = &RCC->APB1RSTR;
   volatile uint32_t *SPI3PeripheralClockEnable = &RCC->APB1ENR;
   volatile uint32_t *SPI3PeripheralReset = &RCC->APB1RSTR;
   uint8_t SPI1PeripheralIndex = 12U;
   uint8_t SPI2PeripheralIndex = 14U;
   uint8_t SPI3PeripheralIndex = 15U;
}

Spi::Spi(Instance instance, GpioInterface& chipSelectPin):
   mChipSelectPin(chipSelectPin)
{
   SatoruAssert(!aSpiInstance[instance]);
   aSpiInstance[instance] = this;
   mChipSelectPin.ConfigureAsOutput(GpioInterface::PinState::High);

   switch (instance)
   {
   case Spi1:
      mSpiRegisters = SPI1;
      clocks::ResetPeripheral(SPI1PeripheralReset,
                                     SPI1PeripheralIndex);
      clocks::EnablePeripheralClock(SPI1PeripheralClockEnable,
                                           SPI1PeripheralIndex);
      break;
   case Spi2:
      mSpiRegisters = SPI2;
      clocks::ResetPeripheral(SPI2PeripheralReset,
                                     SPI2PeripheralIndex);
      clocks::EnablePeripheralClock(SPI2PeripheralClockEnable,
                                           SPI2PeripheralIndex);
      break;
   case Spi3:
      mSpiRegisters = SPI3;
      clocks::ResetPeripheral(SPI3PeripheralReset,
                                     SPI3PeripheralIndex);
      clocks::EnablePeripheralClock(SPI3PeripheralClockEnable,
                                           SPI3PeripheralIndex);
      break;
   default:
      FATAL();
   }

   InitializeMasterMode();
}

void Spi::InitializeMasterMode()
{
   mSpiRegisters->CR1 |= ((BaudRateDividerLevel << 3) & SPI_CR1_BR);

   // Set up SPI mode 3 (the flash driver defaults to it)
   mSpiRegisters->CR1 |= SPI_CR1_CPOL;
   mSpiRegisters->CR1 |= SPI_CR1_CPHA;
   mSpiRegisters->CR1 |= SPI_CR1_SSM;

   // 8 bit frames
   mSpiRegisters->CR1 &= ~SPI_CR1_DFF;

   // MSB First
   mSpiRegisters->CR1 &= ~SPI_CR1_LSBFIRST;

   // No TI mode
   mSpiRegisters->CR2 &= ~SPI_CR2_FRF;

   // Master mode and general enable
   mSpiRegisters->CR2 |= SPI_CR2_SSOE;
   mSpiRegisters->CR1 |= SPI_CR1_MSTR;
   mSpiRegisters->CR1 |= SPI_CR1_SPE;
}

SpiInterface::OperationResult Spi::Enable()
{
   mChipSelectPin.SetOutputLow();
   return Success;
}

SpiInterface::OperationResult Spi::Disable()
{
   if (!WaitForTxe()) {
      debug::PrintBlocking("Spi::Disable() txe timeout\r\n");
      return Failure;
   }

   if (!WaitUntilNotBusy()) {
      debug::PrintBlocking("Spi::Disable() busy timeout\r\n");
      return Failure;
   }

   mChipSelectPin.SetOutputHigh();
   return Success;
}

SpiInterface::OperationResult Spi::Transmit(uint8_t byte)
{
   bool txeSuccess = WaitForTxe();
   if (!txeSuccess) {
      debug::PrintBlocking("Spi::Transmit() txe timeout\r\n");
      return Failure;
   }

   // Write byte
   mSpiRegisters->DR = byte;
   return Success;
}

SpiInterface::OperationResult Spi::Receive(uint8_t &byte) const
{
   bool rxneSuccess = WaitForRxne();
   if (!rxneSuccess) {
      debug::PrintBlocking("Spi::Receive() rxne timeout\r\n");
      return Failure;
   }

   // Read byte
   byte = mSpiRegisters->DR;
   return Success;
}

bool Spi::WaitForTxe() const
{
   const uint32_t ChecksBeforeMsWait = 100U;
   uint32_t msToWait = 5U; 

   for (uint32_t i=0; i!=msToWait; i++) { 
      for (uint32_t check=0U; check<ChecksBeforeMsWait; check++) {
         const bool txe = (mSpiRegisters->SR & SPI_SR_TXE);
         if (txe) {
            return true;
         }
      }
      WaitMs(1U);
   }

   return false;  // timeout
}

bool Spi::WaitForRxne() const
{
   const uint32_t ChecksBeforeMsWait = 100U;
   uint32_t msToWait = 5U; 

   for (uint32_t i=0; i!=msToWait; i++) { 
      for (uint32_t check=0U; check<ChecksBeforeMsWait; check++) {
         const bool rxne = (mSpiRegisters->SR & SPI_SR_RXNE);
         if (rxne) {
            return true;
         }
      }
      WaitMs(1U);
   }

   return false;  // timeout
}

bool Spi::WaitUntilNotBusy() const
{
   const uint32_t ChecksBeforeMsWait = 100U;
   uint32_t msToWait = 5U; 
   
   for (uint32_t i=0; i!=msToWait; i++) { 
      for (uint32_t check=0U; check<ChecksBeforeMsWait; check++) {
         bool busy = (mSpiRegisters->SR & SPI_SR_BSY);
         if (!busy) {
            return true;
         }
      }
      WaitMs(1U);
   }

   return false;  // timeout
}

} }
