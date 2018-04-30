#include "IOPinMap.h"
#include "Clocks.h"
#include "Gpio.h"

#ifdef STM32F7
#include "stm32f7xx.h"
#else
#include "stm32f4xx.h"
#endif

using namespace ::stm32_drivers;

namespace sandbox { 

namespace
{
   const uint8_t TIM9AlternateFunction = 3u;
   const uint8_t UART3AlternateFunction = 7u;
   const uint8_t USART6AlternateFunction = 8u;

   const PeripheralIoPin PeripheralIoPins[] = {
      // UART3 / ST-Link VCOM
      { { &IoPortD,  8U }, UART3AlternateFunction }, // Tx
      { { &IoPortD,  9U }, UART3AlternateFunction }, // Rx
      // UART6
      { { &IoPortG,  14U }, USART6AlternateFunction }, // Tx
      { { &IoPortG,  9U }, USART6AlternateFunction }, // Rx
   };

   const uint8_t NumberOfPeripheralIoPins = sizeof(PeripheralIoPins) / sizeof(PeripheralIoPin);
}

static void ConfigurePeripheralPins()
{
   for(uint32_t i = 0U; i < NumberOfPeripheralIoPins; i++) {
      // if clock not enabled for port, initialise it completely
      const IoPort *Port = PeripheralIoPins[i].Pin.Port;
      if(((*Port->PeripheralClockEnable) & (1U << Port->PeripheralIndex)) == 0U) {
         Clocks::EnablePeripheralClock(Port->PeripheralClockEnable, Port->PeripheralIndex);
      }
      Gpio::SetGpioMode(&(Port->Registers->MODER), static_cast<uint8_t>(GpioMode::AlternateFunction), PeripheralIoPins[i].Pin.PinIndex);
      Gpio::SetAlternateFunction(Port->Registers->AFR, PeripheralIoPins[i].AlternateFunction, PeripheralIoPins[i].Pin.PinIndex);
   }
}

IOPinMap::IOPinMap()
{
   ConfigurePeripheralPins();
}

}
