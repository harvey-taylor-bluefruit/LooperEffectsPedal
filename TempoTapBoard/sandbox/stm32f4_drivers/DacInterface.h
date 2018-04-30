#ifndef DAC_INTERFACE_H
#define DAC_INTERFACE_H

#include <stdint.h>
namespace DACChannels {
   enum Channel {
      Channel1 = 0u,
      Channel2
   };

   static const uint8_t ChannelCount = 2u;
   static const uint16_t WorkingElectrodePotential = 300u;
   static const uint16_t AmplifierOffset = 925u;
}

class DacInterface
{
public:
   virtual ~DacInterface() {};

   virtual void Enable() = 0;
};

#endif
