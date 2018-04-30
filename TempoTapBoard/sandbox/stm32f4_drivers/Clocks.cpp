#include "Clocks.h"
#include "stm32f4xx.h"
#include "SatoruAssert.h"

namespace stm32_drivers {

const uint32_t Clocks::SystemClockFrequencyHz = 180000000U;
const uint32_t Clocks::PeripheralBusAPB1ClockFrequencyHz = Clocks::SystemClockFrequencyHz / 4U;
const uint32_t Clocks::PeripheralBusAPB2ClockFrequencyHz = Clocks::SystemClockFrequencyHz / 2U;

static void ConfigurePLL()
{
   // pll input clock = 16mhz (HSI, internal crystal)

    // f(vco clock) = pll input clock * (plln / pllm)
    // f(vco input) = pll input clock / pllm
    // f(sys clock) = f(vco clock) / pllp

    // vco input should be 1-2mhz
    // vco output frequency should be between 100 and 432mhz

/* Need this if switch to external crystal...
 *
   RCC->CR |= RCC_CR_HSEON;
   while (!(RCC->CR & RCC_CR_HSERDY)) {
      __NOP();
   }
 *
 */

   const uint32_t plln = 360;
   const uint32_t pllm = 16U;
   const uint32_t pllq = 8U;       // divisor for other clocks to get 48MHz
   const uint32_t pllp = 0U;       // pllp divisor = 2
   const uint32_t pllSource = 0U;  // pll source is HSE (external crystal)

   uint32_t pllCfgr = RCC->PLLCFGR;
   pllCfgr &= 0xF0BC8000;          // mask reserved values and set others to 0

   pllCfgr |= (pllq << 24) | (pllp << 16) | (plln << 6) | (pllm << 0) | (pllSource << 22);
   RCC->PLLCFGR = pllCfgr;

   const uint32_t pllOn = 1U << 24;
   RCC->CR |= pllOn;

   // Wait until it's ready
   RCC->CR |= RCC_CR_PLLON;
   while (!(RCC->CR & RCC_CR_PLLRDY)) {
      __NOP();
   }
}

static void SwitchSystemClockToPLL()
{
   uint32_t cfgr = RCC->CFGR;

   // Configure Flash prefetch, Instruction cache, Data cache and wait state
   uint32_t flashConfig = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_PRFTEN|FLASH_ACR_LATENCY_5WS;
   FLASH->ACR = flashConfig;
   while((FLASH->ACR & flashConfig) != flashConfig) { // read back - see reference manual
      __NOP();
   }

   // Use the PLL to derive the clock speed
   const uint32_t setPLLClockSource = 2U;
   const uint32_t clockSourceClearMask = 0xFFFFFFFC;
   cfgr &= clockSourceClearMask;
   cfgr |= setPLLClockSource;

   RCC->CFGR = cfgr;

   // Wait until PLL used the system clock
   const uint32_t getPLLClockSource = 2 << 2U;
   while(!(RCC->CFGR & getPLLClockSource)) {
      __NOP();
   }

/* Would need this to turn off the HSI (if switch to
   external crystal)
 *
   RCC->CR &= ~RCC_CR_HSION;
   while (RCC->CR & RCC_CR_HSION) {
      __NOP();
   }
 */
}

static void ConfigurePeripheralClocks()
{
   uint32_t cfgr = RCC->CFGR;

   // AHB prescaling...
   // "0xxx: system clock not divided"
   const uint32_t AHBPrescaler = 0U;
   const uint32_t AHBPrescalerPosition = 4U;
   const uint32_t hpreClearMask = 0xFFFFFF0F;
   cfgr &= hpreClearMask;
   cfgr |= (AHBPrescaler << AHBPrescalerPosition);

   // Max 42 mhz APB1 clock
   const uint32_t APB1PrescalerPosition = 10U;
   const uint32_t APB1Prescaler = 5U;     // "101 == AHB clock divided by 4"

   // Max 84 mhz APB2 clock
   const uint32_t APB2PrescalerPosition = 13U;
   const uint32_t APB2Prescaler  = 4U;           // "100 == AHB clock divided by 2"

   const uint32_t prescalarClearMask = 0xFFFF0CFF;
   cfgr &= prescalarClearMask;
   cfgr |= ((APB1Prescaler << APB1PrescalerPosition) | (APB2Prescaler << APB2PrescalerPosition));
   RCC->CFGR = cfgr;

   while(RCC->CFGR != cfgr) {  // read back - see reference manual
      __NOP();
   }
}

Clocks::Clocks()
{
   static bool systemClockConfigured = false;
   SatoruAssert(!systemClockConfigured);

   ConfigurePLL();
   ConfigurePeripheralClocks();
   SwitchSystemClockToPLL();

   systemClockConfigured = true;
}

void Clocks::ResetPeripheral(volatile uint32_t *peripheralReset, uint8_t peripheralIndex)
{
   *peripheralReset |= 1 << peripheralIndex;
   __NOP();
   *peripheralReset &= ~(1 << peripheralIndex);
}

void Clocks::EnablePeripheralClock(volatile uint32_t *peripheralClockEnable, uint8_t peripheralIndex)
{
   *peripheralClockEnable |= 1 << peripheralIndex;
}

void Clocks::DisablePeripheralClock(volatile uint32_t *peripheralClockEnable, uint8_t peripheralIndex)
{
   *peripheralClockEnable &= ~(1 << peripheralIndex);
}

bool Clocks::IsPeripheralClockEnabled(volatile uint32_t *peripheralClockEnable, uint8_t peripheralIndex)
{
   return (((*peripheralClockEnable) & (1U << peripheralIndex)) != 0U);
}

}
