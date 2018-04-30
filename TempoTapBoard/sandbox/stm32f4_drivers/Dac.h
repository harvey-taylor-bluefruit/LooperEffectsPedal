#ifndef DAC_H
#define DAC_H

#include "DacInterface.h"
#include "MvToDacLevelConverter.h"

namespace sandbox { namespace STM32Drivers {

class Dac : public DacInterface
{
public:
   explicit Dac(MvToDacLevelConverter &mvToDacLevelConverter);
   void Enable();

private:
   void ConfigureClocks() const;
   void SetCalibrationData();
   void ConfigureChannels();
   void SetDacChannelOneOutput(uint16_t DacLevel);
   void SetDacChannelTwoOutput(uint16_t DacLevel);
   void ChannelEnable() const;

   MvToDacLevelConverter &mMvToDacLevelConverter;
   DacCalibrationData mDacCalibrationData;
};

} }

#endif
