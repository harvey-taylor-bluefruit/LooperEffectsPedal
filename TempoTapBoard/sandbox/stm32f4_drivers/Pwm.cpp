#include "Pwm.h"
#include "Clocks.h"
#include "stm32f4xx.h"
#include "SatoruAssert.h"

namespace sandbox { namespace STM32Drivers {

namespace
{
   volatile uint32_t *aTIM9PeripheralClockEnable = &RCC->APB2ENR;
   const uint8_t aTIM9PeripheralIndex = 16u;

   const uint32_t aTimer9Frequency_hz = Clocks::PeripheralBusAPB2ClockFrequencyHz * 2;
   const uint32_t aPWMFrequency_hz = 1000u;
   const uint32_t aTimer9Prescaler = 12u;
   const uint32_t aTimer9Period = ((aTimer9Frequency_hz / (aTimer9Prescaler)) / aPWMFrequency_hz);
   const uint16_t aTimer9DefaultPulse = 0u;
   const uint16_t aTimer9MaximumPulseAsPercentage = 100u;

   const uint16_t aCaptureCompare1OutputEnable = 0x01u;
   const uint16_t aCaptureCompare1PreloadEnable = 0x01u;
   const uint16_t aCaptureOutputModePWM = 0x06u;
   const uint16_t aTimerCounterEnable = 0x01u;

   const uint16_t aCaptureCompare1OutputEnablePosition = 0u;
   const uint16_t aCaptureCompare1PreloadPosition = 3u;
   const uint16_t aOutputCompare1ModePosition = 4u;
   const uint16_t aTimerCounterEnablePosition = 0u;
}

Pwm::Pwm()
{
   ConfigurePwm();
}

void Pwm::ConfigurePwm()
{
   SatoruAssert(aTimer9Prescaler < UINT16_MAX);
   SatoruAssert(aTimer9Period < UINT16_MAX);

   Clocks::EnablePeripheralClock(aTIM9PeripheralClockEnable,
                                 aTIM9PeripheralIndex);

   // setup the timer counters
   TIM9->PSC = (aTimer9Prescaler - 1);
   TIM9->ARR = (aTimer9Period - 1);

   // setup the timer output capture for PWM mode
   TIM9->CCMR1 |= aCaptureOutputModePWM << aOutputCompare1ModePosition;
   TIM9->CCER |= aCaptureCompare1OutputEnable << aCaptureCompare1OutputEnablePosition;
   TIM9->CCR1 = aTimer9DefaultPulse;

   // setup preload
   TIM9->CCMR1 |= aCaptureCompare1PreloadEnable << aCaptureCompare1PreloadPosition;

   // enable timer
   TIM9->CR1 |= aTimerCounterEnable << aTimerCounterEnablePosition;
}

void Pwm::SetLevelAsPercent(uint8_t level)
{
   SatoruAssert(level <= aTimer9MaximumPulseAsPercentage);

   uint32_t pwmPulse = 0;

   if (level > 0u) {
      pwmPulse = (level * (aTimer9Period + 1u) / aTimer9MaximumPulseAsPercentage) - 1u;
   }

   SatoruAssert(pwmPulse < UINT16_MAX);

   TIM9->CCR1 = pwmPulse;
}

} }
