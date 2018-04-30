#ifndef ADC_INTERFACE_H
#define ADC_INTERFACE_H

#include <stdint.h>

namespace ADCChannels {
   enum Channel {
      WorkingElectrode1 = 0u,
      WorkingElectrode2,
      WorkingElectrode3,
      WorkingElectrode4,
      VBatt
   };

   static const uint8_t ChannelCount = 5u;
   static const uint8_t WorkingElectrodeCount = 4u;

   static const uint8_t AdcSampleCount = 128u;
   static const uint16_t AdcBufferSize = ADCChannels::ChannelCount * AdcSampleCount;
}

class AdcInterface
{
public:
   virtual ~AdcInterface() {};

   virtual void Enable() = 0;
   virtual uint16_t ReadSingleChannelBlocking(ADCChannels::Channel channel) = 0;
   virtual void ReadAllChannelsBlocking(uint16_t AdcReadings[ADCChannels::ChannelCount]) = 0;
};

#endif
