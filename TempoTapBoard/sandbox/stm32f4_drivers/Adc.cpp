#include "Adc.h"
#include "Clocks.h"
#include "Satoru.h"
#include "InterruptPriorities.h"
#include "CountingSemaphore.h"
#include "stm32f4xx.h"
#include "SatoruAssert.h"

namespace sandbox { namespace STM32Drivers {

namespace
{
   namespace DMARegisters
   {
      const uint32_t EnablePosition = 0u;
      const uint32_t TransmitCompleteInterruptEnablePosition = 4u;
      const uint32_t CircularModePosition = 8u;
      const uint32_t MemoryIncPosition = 10u;
      const uint32_t MemoryDataSizePosition = 11u;
      const uint32_t PeripheralDataSizePosition = 13u;
      const uint32_t PriorityLevelPosition = 16u;

      const uint16_t HalfWord = 0x01;
      const uint16_t HighPriority = 0x02;
   }

   namespace ADCRegisters
   {
      const uint32_t AdcOnPosition = 0u;
      const uint32_t ContinuousConversionPosition = 1u;
      const uint32_t ScanPosition = 8u;
      const uint32_t DMAPosition = 8u;
      const uint32_t DMADisableSelectionPosition = 9u;
      const uint32_t ConversionCountPosition = 20U;
      const uint32_t StartConversionPosition = 30u;

      const uint32_t CycleCount28 = 0x02;

      const uint8_t ChannelFieldWidth = 5u;
      const uint8_t SampleTimeFieldWidth = 3u;
      const uint8_t WE1Channel = 3u;
      const uint8_t WE2Channel = 6u;
      const uint8_t WE3Channel = 7u;
      const uint8_t WE4Channel = 8u;
      const uint8_t VBattChannel = 1u;
   }

   volatile uint32_t *ADC1PeripheralClockEnable = &RCC->AHB1ENR;
   const uint8_t ADC1PeripheralIndex = 22u;
   volatile uint32_t *DMA2PeripheralClockEnable = &RCC->APB2ENR;
   const uint8_t DMA2PeripheralIndex = 8U;

   satoru::CountingSemaphore dataAquisitionSemaphore;
   volatile bool conversionComplete;
   volatile bool isSchedulerRunning;

   extern "C" void DMA2_Stream0_IRQHandler(void)
   {
       if (DMA2->LISR & DMA_LISR_TCIF0) {
         DMA2->LIFCR |= DMA_LIFCR_CTCIF0;    // clear flag
         // disable continous conversion mode so that ADC stops sampling
         ADC1->CR2 &= ~(0x01u << ADCRegisters::ContinuousConversionPosition);

         if (isSchedulerRunning) {
            dataAquisitionSemaphore.Give();
         } else {
            conversionComplete = true;
         }
       }
   }
}

Adc::Adc(AdcDataStore &adcDataStore) :
      mAdcDataStore(adcDataStore)
{
}

void Adc::Enable()
{
   ConfigureClocks();
   ConfigureChannels();
   TurnOn();
}

uint16_t Adc::ReadSingleChannelBlocking(ADCChannels::Channel channel)
{
   SatoruAssert(channel < ADCChannels::ChannelCount);

   PerformDataAcquisitionBlocking();
   mAdcDataStore.OversampleRawData();

   return mAdcDataStore.OversampledValues[channel];
}

void Adc::ReadAllChannelsBlocking(uint16_t AdcReadings[ADCChannels::ChannelCount])
{
   PerformDataAcquisitionBlocking();
   mAdcDataStore.OversampleRawData();

   uint32_t i;
   for (i=0; i<ADCChannels::ChannelCount; i++) {
      AdcReadings[i] = mAdcDataStore.OversampledValues[i];
   }
}

void Adc::ConfigureClocks()
{
   Clocks::EnablePeripheralClock(DMA2PeripheralClockEnable,
                                 DMA2PeripheralIndex);
   Clocks::EnablePeripheralClock(ADC1PeripheralClockEnable,
                                 ADC1PeripheralIndex);

   uint8_t AdcClockPrescaler;
   if (Clocks::PeripheralBusAPB2ClockFrequencyHz <= 72) { // max ADCCLK is 36MHz
      AdcClockPrescaler = 0; // divide by 2
   } else {
      AdcClockPrescaler = 1; // divide by 4
   }
   const uint32_t AdcPrescalerPosition = 16U;
   ADC->CCR |= AdcClockPrescaler << AdcPrescalerPosition;
}

void Adc::ConfigureChannels()
{
   // setup DMA
   DMA2_Stream0->CR |= 0x01 << DMARegisters::TransmitCompleteInterruptEnablePosition;
   DMA2_Stream0->CR |= 0x01 << DMARegisters::CircularModePosition;
   DMA2_Stream0->CR |= 0x01 << DMARegisters::MemoryIncPosition;
   DMA2_Stream0->CR |= DMARegisters::HighPriority << DMARegisters::PriorityLevelPosition;
   DMA2_Stream0->CR |= DMARegisters::HalfWord << DMARegisters::MemoryDataSizePosition;
   DMA2_Stream0->CR |= DMARegisters::HalfWord << DMARegisters::PeripheralDataSizePosition;

   DMA2_Stream0->NDTR = ADCChannels::AdcBufferSize;
   DMA2_Stream0->PAR = (uint32_t) &ADC1->DR;
   NVIC_SetPriority(DMA2_Stream0_IRQn, InterruptPriorities::DMA);
   NVIC_EnableIRQ(DMA2_Stream0_IRQn);
   DMA2_Stream0->M0AR = (uint32_t) &mAdcDataStore.RawValues;

   NVIC_SetPriority(DMA2_Stream0_IRQn, InterruptPriorities::DMA);
   NVIC_EnableIRQ(DMA2_Stream0_IRQn);

   DMA2_Stream0->CR |= 0x01 << DMARegisters::EnablePosition;

   // setup channels
   ADC1->SQR3 |= ADCRegisters::WE1Channel << (ADCRegisters::ChannelFieldWidth * 0);
   ADC1->SQR3 |= ADCRegisters::WE2Channel << (ADCRegisters::ChannelFieldWidth * 1);
   ADC1->SQR3 |= ADCRegisters::WE3Channel << (ADCRegisters::ChannelFieldWidth * 2);
   ADC1->SQR3 |= ADCRegisters::WE4Channel << (ADCRegisters::ChannelFieldWidth * 3);
   ADC1->SQR3 |= ADCRegisters::VBattChannel << (ADCRegisters::ChannelFieldWidth * 4);

   ADC1->SMPR2 |= ADCRegisters::CycleCount28 << (ADCRegisters::SampleTimeFieldWidth * ADCRegisters::WE1Channel);
   ADC1->SMPR2 |= ADCRegisters::CycleCount28 << (ADCRegisters::SampleTimeFieldWidth * ADCRegisters::WE2Channel);
   ADC1->SMPR2 |= ADCRegisters::CycleCount28 << (ADCRegisters::SampleTimeFieldWidth * ADCRegisters::WE3Channel);
   ADC1->SMPR2 |= ADCRegisters::CycleCount28 << (ADCRegisters::SampleTimeFieldWidth * ADCRegisters::WE4Channel);
   ADC1->SMPR2 |= ADCRegisters::CycleCount28 << (ADCRegisters::SampleTimeFieldWidth * ADCRegisters::VBattChannel);

   ADC1->SQR1 |= (ADCChannels::ChannelCount - 1U) << ADCRegisters::ConversionCountPosition;

   ADC1->CR1 |= 0x01 << ADCRegisters::ScanPosition;

   ADC1->CR2 |= 0x01 << ADCRegisters::DMADisableSelectionPosition;
   ADC1->CR2 |= 0x01 << ADCRegisters::DMAPosition;
}

void Adc::TurnOn()
{
   ADC1->CR2 |= 0x01 << ADCRegisters::AdcOnPosition;
}

void Adc::PerformDataAcquisitionBlocking()
{
   // As the ADC will be accessed before the Scheduler is running we need to
   // Ensure that we take a different approach if the Scheduler is not running
   isSchedulerRunning = satoru::core::IsRunning();
   conversionComplete = false;

   // enable the continous conversion before starting a scan
   // this is because the DMA buffer is 128 timers bigger than the
   // number of scans the ADC will perform each time. if you dont
   // the DMA interrupt will never fire and you'll sit here forever
   ADC1->CR2 |= 0x01u << ADCRegisters::ContinuousConversionPosition;
   ADC1->CR2 |= 0x01u << ADCRegisters::StartConversionPosition;

   if (isSchedulerRunning) {
      dataAquisitionSemaphore.Take();
   } else {
      while(!conversionComplete) {};
   }
}

} }
