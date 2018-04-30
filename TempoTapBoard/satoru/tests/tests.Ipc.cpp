#include "gtest/gtest.h"
#include "SatoruConfigSample.h"

#include "EventManager.h"
#include "EventConsumer.h"

#include "Satoru.h"

#include "../satoru/internals/Resources.h"
#include "../satoru/internals/Thread.h"

#include "../Mocks/mock.Context.h"
#include "../Mocks/mock.Critical.h"
#include "../Mocks/mock.Satoru.Port.h"

typedef struct TestEvent {
   size_t eventData;
} TestEvent;

namespace
{
   void ArbitraryThreadMain(void)
   {}
}

using namespace ::satoru;
using namespace ::satoru::config;

class IpcTests : public ::testing::Test
{
public:
   static const size_t MockStackSize = 100;

   IpcTests() {
      MockCritical_Initialise();
      MockContext_Initialise();
      MockSatoruPort::Initialise();
      uint8_t mockStack[MockStackSize];
      resources.Reset();
      satoru::core::CreateThread(mockStack, sizeof(mockStack), ArbitraryThreadMain);
      satoru::core::Start();
   }

   EventConsumer<TestEvent> mConsumer;
   EventConsumer<TestEvent> mAnotherConsumer;
};

TEST_F(IpcTests, no_event_if_nothing_raised)
{
   ASSERT_FALSE(mConsumer.EventsAvailable());
}

TEST_F(IpcTests, get_event_retrieves_last_raised_event)
{
   // Given
   const size_t arbitraryData = 5;
   TestEvent eventToRaise;
   eventToRaise.eventData = arbitraryData;

   // When
   EventManager::RaiseEvent<TestEvent>(eventToRaise);

   // Then
   ASSERT_TRUE(mConsumer.EventsAvailable());
   ASSERT_EQ(mConsumer.GetEvent().eventData, arbitraryData);
   ASSERT_FALSE(mConsumer.EventsAvailable()); // The event was consumed by this consumer.
}

TEST_F(IpcTests, polling_multiple_events_in_succession)
{
   // Given
   const size_t numberOfEvents = 5;
   TestEvent eventToRaise[numberOfEvents];

   // When
   for (size_t i = 0; i < numberOfEvents; i++) {
      eventToRaise[i].eventData = i;
      EventManager::RaiseEvent<TestEvent>(eventToRaise[i]);
   }

   for (size_t i = 0; i < numberOfEvents; i++) {
      ASSERT_TRUE(mConsumer.EventsAvailable());
      ASSERT_EQ(mConsumer.GetEvent().eventData, i);
   }

   ASSERT_FALSE(mConsumer.EventsAvailable()); // The event was consumed by this consumer.
}

TEST_F(IpcTests, each_consumer_accesses_the_same_event_once)
{
   // Given
   const size_t arbitraryData = 5;
   TestEvent eventToRaise;
   eventToRaise.eventData = arbitraryData;

   // When
   EventManager::RaiseEvent<TestEvent>(eventToRaise);

   // Then
   ASSERT_TRUE(mConsumer.EventsAvailable());
   ASSERT_TRUE(mAnotherConsumer.EventsAvailable());

   // When
   ASSERT_EQ(mConsumer.GetEvent().eventData, arbitraryData); // First consumer takes the event

   // Then
   ASSERT_FALSE(mConsumer.EventsAvailable());
   ASSERT_TRUE(mAnotherConsumer.EventsAvailable());

   // When
   ASSERT_EQ(mAnotherConsumer.GetEvent().eventData, arbitraryData); // Second consumer takes the event

   // Then
   ASSERT_FALSE(mConsumer.EventsAvailable());
   ASSERT_FALSE(mAnotherConsumer.EventsAvailable());
}

TEST_F(IpcTests, consumers_for_different_event_types_do_not_interfere_with_each_other)
{
   // Given
   EventConsumer<int32_t> intConsumer;
   EventConsumer<float> floatConsumer;
   const float expectedFloat = 0.5;
   const int32_t expectedInt = 5;

   // When
   EventManager::RaiseEvent<float>(expectedFloat);

   // Then
   ASSERT_FALSE(intConsumer.EventsAvailable());
   ASSERT_TRUE(floatConsumer.EventsAvailable());

   // When
   EventManager::RaiseEvent<int32_t>(expectedInt);

   // Then
   ASSERT_TRUE(intConsumer.EventsAvailable());
   ASSERT_TRUE(floatConsumer.EventsAvailable());

   // When
   ASSERT_EQ(intConsumer.GetEvent(), expectedInt);

   // Then
   ASSERT_FALSE(intConsumer.EventsAvailable());
   ASSERT_TRUE(floatConsumer.EventsAvailable());

   // When
   ASSERT_EQ(floatConsumer.GetEvent(), expectedFloat);

   // Then
   ASSERT_FALSE(intConsumer.EventsAvailable());
   ASSERT_FALSE(floatConsumer.EventsAvailable());
}

TEST_F(IpcTests, overflowing_the_event_queue_before_a_consumer_reads_it_marks_the_consumer_as_having_missed_events)
{
   TestEvent eventToRaise;
   eventToRaise.eventData = 0;

   // When
   for (size_t i = 0; i < QueueSize<TestEvent>::value - 1; i++) {
      EventManager::RaiseEvent<TestEvent>(eventToRaise);
   }

   // Then
   ASSERT_FALSE(mConsumer.MissedAnyEvents());

   // When
   EventManager::RaiseEvent<TestEvent>(eventToRaise);

   // Then
   ASSERT_TRUE(mConsumer.MissedAnyEvents());
   ASSERT_FALSE(mConsumer.MissedAnyEvents()); // Cleared on check

}

TEST_F(IpcTests, overflowing_the_event_queue_for_one_consumer_does_not_affect_the_rest)
{
   // Given
   const size_t eventsPastOverflow = 3;
   const size_t numberOfEvents = QueueSize<TestEvent>::value + eventsPastOverflow;

   // When
   for (size_t i = 0; i < numberOfEvents; i++) {
      TestEvent eventToRaise;
      eventToRaise.eventData = i;
      EventManager::RaiseEvent<TestEvent>(eventToRaise);
      mConsumer.GetEvent();
   }
   ASSERT_FALSE(mConsumer.EventsAvailable());

   // Then the other consumer misses the oldest three events (plus another one because of head->tail separation)
   for (size_t i = 0; i < QueueSize<TestEvent>::value - 1 ; i++) {
      ASSERT_TRUE(mAnotherConsumer.EventsAvailable());
      ASSERT_EQ(mAnotherConsumer.GetEvent().eventData, i + 4);
   }
   ASSERT_FALSE(mAnotherConsumer.EventsAvailable());
}

TEST_F(IpcTests, new_subscriber_by_default_does_not_get_notified_of_previously_raised_events)
{
   // Given
   TestEvent eventToRaise;
   EventManager::RaiseEvent<TestEvent>(eventToRaise);

   // When
   EventConsumer<TestEvent> consumer;

   // Then
   ASSERT_FALSE(consumer.EventsAvailable());
}

TEST_F(IpcTests, new_subscriber_does_optionally_get_notified_of_previously_raised_events)
{
   // Given
   TestEvent eventToRaise;
   EventManager::RaiseEvent<TestEvent>(eventToRaise);

   // When
   EventConsumer<TestEvent> consumer(true);

   // Then
   ASSERT_TRUE(consumer.EventsAvailable());
   consumer.GetEvent();
   ASSERT_FALSE(consumer.EventsAvailable());
}

TEST_F(IpcTests, event_consumer_can_clear_all_pending_events)
{
   // Given
   TestEvent eventToRaise;
   EventManager::RaiseEvent<TestEvent>(eventToRaise);
   EventManager::RaiseEvent<TestEvent>(eventToRaise);
   ASSERT_TRUE(mConsumer.EventsAvailable());

   // When
   mConsumer.ClearEvents();

   // Then
   ASSERT_FALSE(mConsumer.EventsAvailable());

   EventManager::RaiseEvent<TestEvent>(eventToRaise);
   ASSERT_TRUE(mConsumer.EventsAvailable());
}
