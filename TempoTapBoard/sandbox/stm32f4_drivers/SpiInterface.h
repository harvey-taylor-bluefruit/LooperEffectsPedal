#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include <stdint.h>
#include "ByteReceiveInterruptCallbackInterface.h"

/* This is not an interface for a generalised driver. Only options relevant
 * to the current design are exposed; everything else defaults to the common
 * denominator between the SPI users.*/
class SpiInterface
{
public:
   enum OperationResult {
       Success = 0,
       Failure,
       Busy
   };

   virtual ~SpiInterface() {};

   virtual OperationResult Transmit(uint8_t byte) = 0;
   virtual OperationResult Receive(uint8_t &byte) const = 0;
   virtual OperationResult Enable() = 0;
   virtual OperationResult Disable() = 0;
};

#endif
