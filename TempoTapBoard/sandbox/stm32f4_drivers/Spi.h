#ifndef SPI_H
#define SPI_H

#include "SpiInterface.h"
#include "stm32f4xx.h"
#include "GpioInterface.h"

#include <stdint.h>

namespace sandbox { namespace STM32Drivers {

class Spi: public SpiInterface
{
public:
   enum Instance {
      Spi1,
      Spi2,
      Spi3,
      NumberOfInstances
   };

   // Reception callback is triggered at the end of an exchange,  with
   // the received byte as an argument. It must be lean and ISR-friendly.
   Spi(Instance, GpioInterface& chipSelectPin);

   OperationResult Enable();
   OperationResult Disable();
   OperationResult Transmit(uint8_t byte);
   OperationResult Receive(uint8_t &byte) const;

private:
   SPI_TypeDef* mSpiRegisters;
   GpioInterface &mChipSelectPin;

   void InitializeMasterMode();
   bool WaitUntilNotBusy() const;
   bool WaitForTxe() const;
   bool WaitForRxne() const;

   static const uint8_t BaudRateDividerLevel = 1; // divide by 2^(BaudRateDividerLevel + 1)
};

} }

#endif
