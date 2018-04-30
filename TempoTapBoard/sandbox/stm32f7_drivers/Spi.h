// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SPI_H
#define SPI_H

#include "SpiInterface.h"
#include "GpioInterface.h"
#include "stm32f7xx.h"

namespace stm32_drivers
{

class Spi: public SpiInterface
{
public:
   enum class Instance {
      Spi1,
      Spi3,
      Spi4,
      Count
   };

   Spi(Instance, GpioInterface& chipSelectPin, SpiMode mode, bool txOnly = false);

   Spi(const Spi&) = delete;
   Spi& operator = (const Spi&) = delete;
   Spi(Spi &&) = delete;
   Spi& operator = (Spi&& other) = delete;

   void Enable() override;
   bool Disable() override;
   bool TransmitAndReceive(uint8_t tx, uint8_t &rx) override; // Not available on TxOnly configuration
   bool Transmit(uint8_t tx) override;

private:
   SPI_TypeDef* mSpiRegisters;
   GpioInterface &mChipSelectPin;
   SpiMode mMode;
   bool mTxOnly;

   void InitializeMasterMode();
   bool WaitUntilNotBusy() const;
   bool WaitForTxe() const;

   // baud rate = peripheral clock / 2^(BaudRateDividerLevel + 1)
   static const uint8_t BaudRateDividerLevel = 4u;
};

}

#endif
