#ifndef ADC_H
#define ADC_H

#include "AdcInterface.h"
#include "AdcDataStore.h"

namespace sandbox { namespace STM32Drivers {

class Adc : public AdcInterface
{
public:
   explicit Adc(AdcDataStore &adcDataStore);
   void Enable();
   uint16_t ReadSingleChannelBlocking(ADCChannels::Channel channel);
   void ReadAllChannelsBlocking(uint16_t AllChannels[ADCChannels::ChannelCount]);

private:
   void ConfigureClocks();
   void ConfigureChannels();
   void ConfigureDMA();
   void TurnOn();

   void PerformDataAcquisitionBlocking();

   AdcDataStore &mAdcDataStore;
};

} } 

#endif
