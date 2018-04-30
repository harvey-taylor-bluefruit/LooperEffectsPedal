#include "Dac.h"
#include "Clocks.h"
#include "stm32f4xx.h"

namespace sandbox { namespace STM32Drivers {

namespace
{
   volatile uint32_t *DACPeripheralClockEnable = &RCC->APB1ENR;
   uint8_t DACPeripheralIndex = 29U;

   const uint32_t OutputBufferDisable = (1u << 1u);
   const uint32_t DACChannelEnable = (1u << 0u);

   const uint32_t DACChannel1 = 0u;
   const uint32_t DACChannel2 = (1u << 4u);

   const float DACUncalibratedSlope = 2.0f;
   const float DACUncalibratedOffset = 0.0f;
   const uint32_t DACUncalibratedReference = 300u;
}

Dac::Dac(MvToDacLevelConverter &mvToDacLevelConverter) :
      mMvToDacLevelConverter(mvToDacLevelConverter)
{
   mDacCalibrationData.Slope = DACUncalibratedSlope;
   mDacCalibrationData.Offset = DACUncalibratedOffset;
   mDacCalibrationData.Reference = DACUncalibratedReference;
}

void Dac::Enable()
{
   ConfigureClocks();
   SetCalibrationData();
   ConfigureChannels();
   ChannelEnable();
}

void Dac::ConfigureClocks() const
{
   Clocks::EnablePeripheralClock(DACPeripheralClockEnable,
                                 DACPeripheralIndex);
}

void Dac::SetCalibrationData()
{
   mDacCalibrationData.Slope = DACUncalibratedSlope;
   mDacCalibrationData.Offset = DACUncalibratedOffset;
   mDacCalibrationData.Reference = DACUncalibratedReference;

   mMvToDacLevelConverter.SetCalibrationData(mDacCalibrationData);
}

void Dac::ConfigureChannels()
{
   DAC->CR |= OutputBufferDisable << DACChannel1;
   DAC->CR |= OutputBufferDisable << DACChannel2;

   uint16_t amplifierOneDac = mMvToDacLevelConverter.PerformConversion(DACChannels::WorkingElectrodePotential, DACChannels::Channel1);
   uint16_t amplifierTwoDac = mMvToDacLevelConverter.PerformConversion(DACChannels::AmplifierOffset, DACChannels::Channel2);

   SetDacChannelOneOutput(amplifierOneDac);
   SetDacChannelTwoOutput(amplifierTwoDac);
}

void Dac::ChannelEnable() const
{
   DAC->CR |= DACChannelEnable << DACChannel1;
   DAC->CR |= DACChannelEnable << DACChannel2;
}

void Dac::SetDacChannelOneOutput(uint16_t DacLevel)
{
   DAC->DHR12R1 = DacLevel;
}

void Dac::SetDacChannelTwoOutput(uint16_t DacLevel)
{
   DAC->DHR12R2 = DacLevel;
}

} }
