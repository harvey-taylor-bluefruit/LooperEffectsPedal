#ifndef ADC_SAMPLER_INTERFACE_H
#define ADC_SAMPLER_INTERFACE_H

#include <stdint.h>

class AdcSamplerInterface {
public:
   enum class Resolution {
      Resolution12bit,
      Resolution10bit,
      Resolution8bit,
      Resolution6bit
   };

   virtual ~AdcSamplerInterface() {}

   virtual uint16_t ReadChannel(uint8_t channelIndex) = 0;
   virtual uint16_t NumberOfChannels() const = 0;

   virtual void Enable(uint8_t channelIndex) = 0;
   virtual void Disable(uint8_t channelIndex) = 0;
   virtual void SetResolution(Resolution) = 0;
};

#endif
