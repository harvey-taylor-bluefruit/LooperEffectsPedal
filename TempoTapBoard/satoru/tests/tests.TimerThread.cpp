#include "gtest/gtest.h"

#include "Satoru.h"
#include "../satoru/internals/Resources.h"
#include "../satoru/internals/Tick.h"
#include "Timer.h"
#include "SatoruPort.h"
#include "SatoruAssert.h"

#include "../Mocks/mock.Context.h"
#include "../Mocks/mock.Critical.h"
#include "../Mocks/mock.Satoru.Port.h"

namespace
{
   size_t timesCallbackCalled = 0;
   size_t timesCallback2Called = 0;
   size_t timesCallback3Called = 0;

   void TestTimerCallback(void)  { timesCallbackCalled++; }
   void TestTimerCallback2(void) { timesCallback2Called++; }
   void TestTimerCallback3(void) { timesCallback3Called++; }

   ThreadStatus GetThreadStatus(ThreadHandle handle)
   {
      SatoruAssert(handle < config::MaxNumberOfThreads);
      return resources.threadPool[handle].Status;
   }
}

namespace satoru
{

extern void ServiceTimerList();

class TimerThreadTests : public ::testing::Test
{
public:
   static const size_t MockStackSize = 256;

   TimerThreadTests() {
      MockCritical_Initialise();
      MockContext_Initialise();
      MockSatoruPort::Initialise();
      timesCallbackCalled = 0;
      timesCallback2Called = 0;
      timesCallback3Called = 0;
   }

   const size_t TimerThreadIndex = 1;
};

TEST_F(TimerThreadTests, timer_thread_is_initialised_as_active)
{
   // When scheduler is initialised
   // Then
   ThreadStatus timerThreadStatus = GetThreadStatus(TimerThreadIndex);
   ASSERT_EQ(ThreadStatus_Active, timerThreadStatus);
}

TEST_F(TimerThreadTests, not_started_timer_does_not_fire)
{
   // Given
   const size_t timeInMs = 1u;
   const bool isRepeating = false;
   Timer timer(TestTimerCallback, timeInMs, isRepeating);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(timeInMs) + 1; tick++){
      satoru::TickHandler();
   }
   ServiceTimerList();

   // Then
   ASSERT_EQ(timesCallbackCalled, 0u);
}

TEST_F(TimerThreadTests, started_timer_fires_after_specified_period_once)
{
   // Given
   const size_t timeInMs = 1u;
   const bool isRepeating = false;
   Timer timer(TestTimerCallback, timeInMs, isRepeating);

   // When
   timer.Start();
   for (size_t tick = 0; tick != port::MsToTicks(timeInMs); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(timeInMs); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u); // Only once
}

TEST_F(TimerThreadTests, repeating_timer_fires_indefinitely)
{
   // Given
   const size_t timeInMs = 5u;
   const size_t timesToRepeat = 3u;
   const bool isRepeating = true;
   Timer timer(TestTimerCallback, timeInMs, isRepeating);

   // When
   timer.Start();
   for (size_t tick = 0; tick != timesToRepeat * port::MsToTicks(timeInMs); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, timesToRepeat);
}

TEST_F(TimerThreadTests, stopping_a_timer_before_firing_prevents_the_callback)
{
   // Given
   const size_t timeInMs = 5u;
   const bool isRepeating = false;
   Timer timer(TestTimerCallback, timeInMs, isRepeating);

   // When
   timer.Start();
   for (size_t tick = 0; tick != port::MsToTicks(timeInMs) - 1; tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }
   timer.Stop();
   satoru::TickHandler();
   ServiceTimerList();

   // Then
   ASSERT_EQ(timesCallbackCalled, 0u);
}

TEST_F(TimerThreadTests, multiple_timers_firing_in_succession)
{
   const size_t interval = 3u;
   // Given
   Timer timer1(TestTimerCallback, interval, false); 
   Timer timer2(TestTimerCallback2, 2 * interval, false); 
   Timer timer3(TestTimerCallback3, 3 * interval, false); 

   // Arbitrary order
   timer3.Start();
   timer1.Start();
   timer2.Start();

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 1u);
} 

TEST_F(TimerThreadTests, multiple_timers_firing_in_succession_with_intermediate_start)
{
   const size_t interval = 3u;
   // Given
   Timer timer1(TestTimerCallback, interval, false); 
   Timer timer2(TestTimerCallback2, 2 * interval, false); 
   Timer timer3(TestTimerCallback3, 3 * interval, false); 
   timer3.Start();
   timer1.Start();

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   timer2.Start();
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 1u);
} 

TEST_F(TimerThreadTests, multiple_timers_firing_in_succession_with_intermediate_stop)
{
   const size_t interval = 3u;
   // Given
   Timer timer1(TestTimerCallback, interval, false); 
   Timer timer2(TestTimerCallback2, 2 * interval, false); 
   Timer timer3(TestTimerCallback3, 3 * interval, false); 
   timer3.Start();
   timer1.Start();
   timer2.Start();

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   timer2.Stop();
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 1u);
} 

TEST_F(TimerThreadTests, multiple_one_shot_timers_alongside_repeating_timer)
{
   const size_t interval = 3u;
   // Given
   Timer repeatingTimer(TestTimerCallback, interval, true); 
   Timer timer2(TestTimerCallback2, 2 * interval, false); 
   Timer timer3(TestTimerCallback3, 3 * interval, false); 

   // Arbitrary order
   timer3.Start();
   repeatingTimer.Start();
   timer2.Start();

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 2u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 3u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 1u);
}

TEST_F(TimerThreadTests, destroying_all_timers_clears_internal_linked_list)
{
   ASSERT_TRUE(resources.firstActiveTimer == 0);
   ASSERT_TRUE(resources.firstPendingTimer == 0);
   {
      Timer timer1(TestTimerCallback, 1, false); 
      Timer timer2(TestTimerCallback2, 2, false); 

      timer2.Start();
      timer1.Start();
      ASSERT_TRUE(resources.firstActiveTimer != 0);
      ASSERT_TRUE(resources.firstPendingTimer != 0);
   }
   ASSERT_TRUE(resources.firstActiveTimer == 0);
   ASSERT_TRUE(resources.firstPendingTimer == 0);
}

TEST_F(TimerThreadTests, multiple_timers_firing_in_succession_with_intermediate_construction_destruction)
{
   const size_t interval = 3u;

   // Given
   Timer timer1(TestTimerCallback, interval, false); 
   Timer timer2(TestTimerCallback2, 2 * interval, false); 
   timer1.Start();
   timer2.Start();

   // When
   { 
      Timer timer3(TestTimerCallback3, interval, true);
      timer3.Start();
      for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
         satoru::TickHandler();
         ServiceTimerList();
      }
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);
   ASSERT_EQ(timesCallback3Called, 1u);


   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 1u);
   ASSERT_EQ(timesCallback3Called, 1u);
}

TEST_F(TimerThreadTests, reconfiguring_stops_timer)
{
   // Given
   const size_t interval = 3u;
   Timer timer(TestTimerCallback2, 1, false); 
   timer.Start();
   timer.Reconfigure(TestTimerCallback, interval, true);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }

   // Then
   ASSERT_EQ(timesCallbackCalled, 0u);
   ASSERT_EQ(timesCallback2Called, 0u);

   // When
   timer.Start();
   for (size_t tick = 0; tick != 3 * port::MsToTicks(interval); tick++){
      satoru::TickHandler();
      ServiceTimerList();
   }
   
   // Then
   ASSERT_EQ(timesCallbackCalled, 3u);
   ASSERT_EQ(timesCallback2Called, 0u);
}

TEST_F(TimerThreadTests, starting_timer_when_timer_is_pending)
{
   const size_t interval = 3u;

   // Given
   Timer timer1(TestTimerCallback, interval, false); 
   Timer timer2(TestTimerCallback2, interval, false); 
   timer1.Start();

   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
   } // Timer 1 is pending to fire

   // When
   timer2.Start();
   ServiceTimerList();

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 0u);

   // When
   for (size_t tick = 0; tick != port::MsToTicks(interval); tick++){
      satoru::TickHandler();
   } 
   ServiceTimerList();

   // Then
   ASSERT_EQ(timesCallbackCalled, 1u);
   ASSERT_EQ(timesCallback2Called, 1u);
}

}
