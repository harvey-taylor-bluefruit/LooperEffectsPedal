#ifndef PWM_H
#define PWM_H

#include "PwmInterface.h"

namespace sandbox { namespace STM32Drivers {

class Pwm : public PwmInterface
{
public:
   explicit Pwm();

   void SetLevelAsPercent(uint8_t level);

private:
   void ConfigurePwm();
};

} }

#endif
