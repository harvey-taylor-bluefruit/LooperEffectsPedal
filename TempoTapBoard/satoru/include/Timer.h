// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <stdint.h>

namespace satoru
{

typedef struct TimerEntry TimerEntry;

class Timer {
public:
   typedef void (*Callback)(void);

   Timer(Callback, size_t timeMs, bool repeating);
   ~Timer();

   void Start();
   void Stop();
   bool IsRunning();

   // Stops the timer if active
   void Reconfigure(Callback, size_t timeMs, bool repeating);

private:
   void Schedule();
   const Timer& operator=(const Timer&);
   Timer(const Timer&);

   TimerEntry* mEntry;
   size_t mTimeTicks;
};

}

#endif
