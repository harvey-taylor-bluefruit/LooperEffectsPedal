#include "BlinkTest.h"
#include "Satoru.h"
#include "SatoruPort.h"

#ifdef STM32F7
#include "stm32f7xx.h"
#else
#include "stm32f4xx.h"
#endif
#include "Wait.h"
#include "DataAlign.h"

using namespace ::stm32_drivers;

namespace
{
   const uint32_t TestThreadLed2StackSize = 1024U;
   DATA_ALIGN_8(uint8_t testThreadLed2Stack[TestThreadLed2StackSize]);

   const uint32_t TestThreadLed1StackSize = 1024U;
   DATA_ALIGN_8(uint8_t testThreadLed1Stack[TestThreadLed1StackSize]);

   const uint32_t TestThreadLed3StackSize = 1024U;
   DATA_ALIGN_8(uint8_t testThreadLed3Stack[TestThreadLed3StackSize]);

   struct BlinkingLed
   {
      const uint8_t PinIndex;
      bool IsOn;
      const uint32_t TogglePeriodMs;
      uint32_t TogglePeriodTicks;
   };

   BlinkingLed led1 = {
         0U,
         false,
         500U,
         0U
   };

   BlinkingLed led2 = {
         7U,
         false,
         1000U,
         0U
   };

   BlinkingLed led3 = {
         14U,
         false,
         250U,
         0U
   };

   void GpioInit()
   {
      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // enable clock for GPIO port B
   }

   void ToggleLed(BlinkingLed *led)
   {
      if (led->IsOn) {
         static const uint8_t resetIndex = 16;
         GPIOB->BSRR = static_cast<uint32_t>(1U << (resetIndex + led->PinIndex));
      } else {
         GPIOB->BSRR = static_cast<uint32_t>(1U << led->PinIndex);
      }
      led->IsOn = !led->IsOn;
   }
}

static void TestThread(BlinkingLed *led)
{
   GPIOB->MODER |= 0x1 << (led->PinIndex * 2); // output
   for(;;) {
      ToggleLed(led);
      WaitMs(led->TogglePeriodMs);
   }
}

static void TestThreadLed1()
{
   TestThread(&led1);
}

static void TestThreadLed2()
{
   TestThread(&led2);
}

static void TestThreadLed3()
{
   TestThread(&led3);
}

void BlinkTest::Setup()
{
   GpioInit();
   satoru::core::CreateThread("Blink1", testThreadLed2Stack, TestThreadLed2StackSize, TestThreadLed2);
   satoru::core::CreateThread("Blink2", testThreadLed1Stack, TestThreadLed1StackSize, TestThreadLed1);
   satoru::core::CreateThread("Blink3", testThreadLed3Stack, TestThreadLed3StackSize, TestThreadLed3);
}
