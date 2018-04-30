#include "gtest/gtest.h"

#include "../satoru/internals/RecursiveMutex.h"
#include "../satoru/internals/Resources.h"
#include "../satoru/internals/Thread.h"
#include "../satoru/internals/Tick.h"

#include "CountingSemaphore.h"
#include "Gate.h"
#include "Satoru.h"
#include "SatoruAssert.h"
#include "SatoruConfig.h"

#include "../Mocks/mock.Context.h"
#include "../Mocks/mock.Critical.h"
#include "../Mocks/mock.Satoru.Port.h"

using namespace ::satoru;

class TestScheduler : public ::testing::Test
{
public:
   static const size_t MockStackSize = 100;

   TestScheduler() {
      MockCritical_Initialise();
      MockContext_Initialise();
      MockSatoruPort::Initialise();
      // The timer thread is disabled for general scheduler tests.
      Scheduler_TimerThread->Status = ThreadStatus_Suspended;
   }

   static const size_t ArbitrarySleepTimeTicks = 50;
   static const size_t IndexOfFirstUserThread = 2;
};

namespace
{
   void ArbitraryThreadMain(void)
   {}

   void ArbitraryThreadMain2(void)
   {}

   void ArbitraryThreadMain3(void)
   {}

   bool AssertCriticalSectionWasEnteredAndLeftNTimes(size_t numTimes)
   {
      if (mockCriticalHistory.EntryCount != (numTimes * 2))
         return false;

      for (size_t i = 0; i < numTimes; i += 2)
      {
         if (mockCriticalHistory.History[i])
            return false;

         if (!mockCriticalHistory.History[i + 1])
            return false;
      }

      return true;
   }

   ThreadStatus GetThreadStatus(ThreadHandle handle)
   {
      SatoruAssert(handle < config::MaxNumberOfThreads);
      return resources.threadPool[handle].Status;
   }

   const char *GetThreadName(ThreadHandle handle)
   {
      SatoruAssert(handle < config::MaxNumberOfThreads);
      return resources.threadPool[handle].Name;
   }
}

TEST_F(TestScheduler, All_user_threads_are_free_after_initialisation)
{
   // Given
   // When
   // .. Scheduler is initialised

   // Then
   for (uint8_t i = IndexOfFirstUserThread; i < config::MaxNumberOfThreads; i++)
   {
      ThreadStatus threadStatus = GetThreadStatus(i);
      ThreadStatus expected = ThreadStatus_Free;
      ASSERT_EQ(expected, threadStatus);
   }
}

TEST_F(TestScheduler, Context_for_the_kernel_thread_is_initialised_with_the_scheduler)
{
   // Given
   // When
   // .. Scheduler is initialised

   // Then
   ASSERT_TRUE(mockContext.StackPointer != NULL);
   ASSERT_TRUE(mockContext.StackSize != 0);
   ASSERT_TRUE(mockContext.EntryPoint != NULL);
   ASSERT_TRUE(mockContext.ExitPoint != NULL);
}

TEST_F(TestScheduler, First_thread_is_not_free_after_it_is_created)
{
   uint8_t mockStack[TestScheduler::MockStackSize];

   // Given
   // .. Scheduler is initialised

   // When
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   // Then... all are free except index returned
   for (uint8_t i = IndexOfFirstUserThread; i < config::MaxNumberOfThreads; i++) {
      ThreadStatus threadStatus = GetThreadStatus(i);

      if (i == IndexOfFirstUserThread) {
         ASSERT_EQ(ThreadStatus_Active, threadStatus);
      } else {
         ASSERT_EQ(ThreadStatus_Free, threadStatus);
      }
   }

   // Then... that thread was created inside a critical section,
   // plus another for the timer and kernel threads
   ASSERT_TRUE(AssertCriticalSectionWasEnteredAndLeftNTimes(3));
}

TEST_F(TestScheduler, Not_running_on_initialisation)
{
   ASSERT_FALSE(satoru::core::IsRunning());
}

TEST_F(TestScheduler, Start_scheduler_sets_it_running)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   // When
   satoru::core::Start();

   // Then
   ASSERT_TRUE(satoru::core::IsRunning());
}

TEST_F(TestScheduler, Start_scheduler_also_causes_an_initial_context_switch_into_the_kernel_thread)
{
   // Given
   // At least one user thread is needed to stop the Start() method from asserting out.
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   // When
   satoru::core::Start();

   // Then
   ASSERT_EQ(mockContext.SwitchToKernelCalls, 1U);
   ASSERT_TRUE(MockContext_IsKernelThreadCurrentThread());
}

TEST_F(TestScheduler, Scheduler_start_calls_scheduler_port_initialisation)
{
   // Given
   ASSERT_EQ(0U, MockSatoruPort::portInitCalls);

   // When
   satoru::core::Start();

   // Then
   ASSERT_EQ(1U, MockSatoruPort::portInitCalls);
}

TEST_F(TestScheduler, Thread_sleep_only_suspends_current_thread)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended, other threads are unaffected

   // Then
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, Single_user_Thread_is_awoken_only_when_sleep_delay_has_elapsed)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended

   // When... sleep delay hasn't elapsed yet
   for(size_t ticks = 0; ticks < ArbitrarySleepTimeTicks - 1; ticks++) {
      satoru::TickHandler();
   //Then... still sleeping
      ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));
   }

   // When... delay elapsed
   satoru::TickHandler();

   // Then.. thread is flagged as active
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
}

TEST_F(TestScheduler, Multiple_user_threads_are_awoken_only_when_sleep_delays_are_elapsed)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 2 should now be suspended
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 3 should now be suspended

   // When... sleep delay hasn't elapsed yet
   for(size_t ticks = 0; ticks < ArbitrarySleepTimeTicks - 1; ticks++) {
      satoru::TickHandler();
   //Then... still sleeping
      ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));
      ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 1));
      ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 2));
   }

   // When... delay elapsed
   satoru::TickHandler();

   // Then.. thread is flagged as active
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, Sleeping_thread_flagged_as_active_only_when_sleep_delay_has_elapsed)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended

   // When... sleep delay hasn't elapsed yet
   mockContext.Switches = 0; // clear history
   for(size_t ticks = 0; ticks < ArbitrarySleepTimeTicks - 1; ticks++) {
      satoru::TickHandler();
   //Then... still sleeping
      ASSERT_EQ(0U, mockContext.Switches);
   }

   // When... delay elapsed
   satoru::TickHandler();

   // Then.. thread is flagged as active
   ASSERT_EQ(1U, mockContext.Switches);
}

TEST_F(TestScheduler, Thread_Sleep_only_suspends_current_thread)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended, other threads are unaffected

   // Then
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, Thread_Sleep_triggers_context_switch)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   mockContext.Switches = 0; // clear history
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // thread 1 should now be suspended

   // Then
   ASSERT_EQ(1U, mockContext.Switches);
}

TEST_F(TestScheduler, Scheduler_selects_first_active_user_thread_to_run)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();

   // When
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // Then user thread selected
   ASSERT_EQ(1U, mockContext.Switches);
   ASSERT_FALSE(MockContext_IsKernelThreadCurrentThread());
   ASSERT_TRUE(mockContext.EntryPoint == ArbitraryThreadMain); // user thread running
}

TEST_F(TestScheduler, Scheduler_only_switches_context_if_required_no_priority)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // suspend current thread

   // When nothing else has happened, run scheduler algorithm again
   mockContext.Switches = 0; // clear history
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // Then user thread selected
   ASSERT_EQ(0U, mockContext.Switches);
}

TEST_F(TestScheduler, Scheduler_algorithm_selects_next_thread_to_run_when_all_thread_have_default_priority)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain2);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain3);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // suspend first user thread
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks); // suspend second user thread

   // Then third user thread is running
   ASSERT_TRUE(mockContext.EntryPoint == ArbitraryThreadMain3);
}

TEST_F(TestScheduler, Scheduler_algorithm_selects_next_thread_to_run_based_on_priority)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1U);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain2, 2U);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain3, 3U);

   // When
   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to user thread with highest priority

   // Then third user thread is running
   ASSERT_TRUE(mockContext.EntryPoint == ArbitraryThreadMain3);
}

TEST_F(TestScheduler, Switch_back_to_kernel_thread_if_all_other_threads_are_suspended)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];

   for(uint8_t i = IndexOfFirstUserThread; i < satoru::config::MaxNumberOfThreads; i ++ ) {
      satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   }

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   for(uint8_t i = IndexOfFirstUserThread; i < satoru::config::MaxNumberOfThreads; i ++ ) {
      satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks);
   }

   // Then
   ASSERT_TRUE(MockContext_IsKernelThreadCurrentThread());
}

TEST_F(TestScheduler, Scheduler_uses_critical_section)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();

   // When
   MockCritical_Initialise(); // clear history
   satoru::core::SelectNextThreadToRun();

   // Then
   ASSERT_TRUE(AssertCriticalSectionWasEnteredAndLeftNTimes(1));
}

TEST_F(TestScheduler, Thread_sleep_switches_to_next_available_thread_with_highest_priority)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1U);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain2, 2U);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain2, 3U);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to user thread with highest priority

   // When
   satoru::core::ThreadSleepTicks(ArbitrarySleepTimeTicks);

   // Then
   ASSERT_TRUE(mockContext.EntryPoint == ArbitraryThreadMain2);
}

TEST_F(TestScheduler, Thread_created_with_lowest_priority_by_default)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain); // default priority

   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain2, satoru::core::LowestPriority);

   // When
   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to user thread with highest priority

   // Then
   ASSERT_TRUE(mockContext.EntryPoint == ArbitraryThreadMain);
}

TEST_F(TestScheduler, taking_binary_mutex_once_changes_ownership_without_suspending) 
{
   // Given
   RecursiveMutex mutex;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread
   mutex.Lock();

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));

   ASSERT_TRUE(mutex.IsLocked());
}

TEST_F(TestScheduler, taking_binary_mutex_twice_from_same_thread_has_no_effect)
{
   // Given
   RecursiveMutex mutex;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   mutex.Lock();
   mutex.Lock();

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));

   ASSERT_TRUE(mutex.IsLocked());
}

TEST_F(TestScheduler, taking_and_giving_mutex_from_second_thread)
{
   // Given
   RecursiveMutex mutex;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 2u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 3u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   mutex.Lock(); // Thread 3
   satoru::core::ThreadSleepTicks(1); // Thread 3 goes to sleep
   mutex.Lock(); // Thread 2

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 2));

   // When
   satoru::TickHandler(); // Thread 3 is awake
   mutex.Unlock();

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));

   ASSERT_FALSE(mutex.IsLocked());
}

TEST_F(TestScheduler, non_owner_can_not_give_mutex_back)
{
   // Given
   RecursiveMutex mutex;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 2u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 3u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   mutex.Lock(); // Thread 3
   satoru::core::ThreadSleepTicks(1); // Thread 3 goes to sleep
   mutex.Lock(); // Thread 2

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 2));

   // When
   mutex.Unlock(); // No effect, since the current thread is not the owner.

   // Then
   ASSERT_TRUE(mutex.IsLocked());
}

TEST_F(TestScheduler, waiting_on_gate_immediately_bLocks_thread)
{
   // Given
   Gate gate;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 2u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 3u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   gate.Wait(); // Thread 3

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 2));

   // When
   gate.Wait();

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, opening_gate_unblocks_all_threads)
{
   // Given
   Gate gate;
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 2u);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 3u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   gate.Wait(); // Thread 3
   gate.Wait(); // Thread 2
   gate.Open(); // Thread 1

   // Then
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, taking_counting_semaphore_blocks_at_count_zero)
{
   // Given
   const size_t initialCount = 3;
   CountingSemaphore semaphore(initialCount);
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   for (size_t i = 0; i != initialCount; i++) {
      semaphore.Take();
      ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   }

   // When
   semaphore.Take();
   // Then
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));
}

TEST_F(TestScheduler, giving_counting_semaphore_raises_count)
{
   // Given
   const size_t initialCount = 0;
   CountingSemaphore semaphore(initialCount);
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   semaphore.Give();
   ASSERT_EQ(semaphore.Count(), initialCount + 1);
}

TEST_F(TestScheduler, giving_counting_semaphore_with_bLocked_thread_unblocks_it_respecting_count)
{
   // Given
   const size_t initialCount = 0;
   CountingSemaphore semaphore(initialCount);
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain, 1u);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to third user thread, because max priority

   // When
   semaphore.Take();

   // Then
   ASSERT_EQ(semaphore.Count(), 0u);
   ASSERT_EQ(ThreadStatus_Suspended, GetThreadStatus(IndexOfFirstUserThread));

   // When
   semaphore.Give();

   // Then
   ASSERT_EQ(semaphore.Count(), 0u);
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(semaphore.Count(), 0u);
}

TEST_F(TestScheduler, Thread_terminate_only_terminates_current_thread)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   satoru::core::ThreadTerminate(); // thread 1 should now be terminated, other threads are unaffected

   // Then
   const size_t FillSize = reinterpret_cast<uint8_t*>(mockContext.StackPointer) - reinterpret_cast<uint8_t*>(mockStack);
   const size_t StackBottomCanarySize = 4U;
   for (size_t i = StackBottomCanarySize; i < FillSize; i++) {
      ASSERT_EQ(0xA5, mockStack[i]);
   }
   ASSERT_EQ(ThreadStatus_Free, GetThreadStatus(IndexOfFirstUserThread));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 1));
   ASSERT_EQ(ThreadStatus_Active, GetThreadStatus(IndexOfFirstUserThread + 2));
}

TEST_F(TestScheduler, Thread_Terminate_triggers_context_switch)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);

   satoru::core::Start();
   satoru::core::SelectNextThreadToRun(); // triggers switch to first user thread

   // When
   mockContext.Switches = 0; // clear history
   satoru::core::ThreadTerminate(); // thread 1 should now be terminated

   // Then
   ASSERT_EQ(1U, mockContext.Switches);
}

TEST_F(TestScheduler, Thread_created_without_a_name_is_given_a_default)
{
   // Given
   uint8_t mockStack[TestScheduler::MockStackSize];
  
   // When
   satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
  
   // Then
   ASSERT_STREQ("[unnamed]", GetThreadName(IndexOfFirstUserThread));
}


