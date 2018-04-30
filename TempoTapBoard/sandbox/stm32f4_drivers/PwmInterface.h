#ifndef PWM_INTERFACE_H
#define PWM_INTERFACE_H

#include <stdint.h>

class PwmInterface
{
public:
   virtual ~PwmInterface() {};

   virtual void SetLevelAsPercent(uint8_t level) = 0;
};

#endif
