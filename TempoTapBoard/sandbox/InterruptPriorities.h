#ifndef INTERRUPT_PRIORITIES_H
#define INTERRUPT_PRIORITIES_H

#include <stdint.h>

namespace InterruptPriorities
{
   const uint8_t LowestPriority = 15;
   const uint8_t HighestPriority = 0;
   const uint8_t SysTickTimer = HighestPriority; // SysTick is a reserved CMSIS symbol
   const uint8_t Uart = 3;
   const uint8_t Spi = 2;
   const uint8_t Usb = 1;
   const uint8_t DMA = 2;
}

#endif
