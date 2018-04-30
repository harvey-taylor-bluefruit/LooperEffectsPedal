#ifndef GPIO_TYPE_DEFINITIONS_H
#define GPIO_TYPE_DEFINITIONS_H

#include "stm32f4xx.h"

namespace stm32_drivers {

namespace GpioTypeDefinitions
{
   struct GpioMode {
      enum Value {
         Input = 0,
         Output = 1,
         AlternateFunction = 2,
         Analog = 3
      };
   };

   struct GpioSpeed {
      enum Value {
         Low = 0,      // 2MHz
         Medium = 1,   // 25MHz
         Fast = 2,     // 50MHz
         High = 3      // 100MHz @ 30pF, 80MHz @ 15pF
      };
   };

   struct GpioPuPd {
      enum Value {
         NoPull = 0,
         Up = 1,
         Down = 2
      };
   };

   struct GpioOutputType {
      enum Value {
         PushPull = 0,
         OpenDrain = 1
      };
   };

   struct IoPort {
      GPIO_TypeDef *Registers;
      volatile uint32_t *PeripheralReset;
      volatile uint32_t *PeripheralClockEnable;
      uint8_t PeripheralIndex;
   };

   struct IoPin {
      const IoPort *Port;
      uint8_t PinIndex;
   };

   struct PeripheralIoPin {
      IoPin Pin;
      uint8_t AlternateFunction;
   };

   const IoPort IoPortA = { GPIOA, &RCC->AHB1RSTR, &RCC->AHB1ENR, 0U };
   const IoPort IoPortB = { GPIOB, &RCC->AHB1RSTR, &RCC->AHB1ENR, 1U };
   const IoPort IoPortC = { GPIOC, &RCC->AHB1RSTR, &RCC->AHB1ENR, 2U };
   const IoPort IoPortD = { GPIOD, &RCC->AHB1RSTR, &RCC->AHB1ENR, 3U };
   const IoPort IoPortE = { GPIOE, &RCC->AHB1RSTR, &RCC->AHB1ENR, 4U };
   const IoPort IoPortG = { GPIOG, &RCC->AHB1RSTR, &RCC->AHB1ENR, 6U };
}

}

#endif
