// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "SatoruAssert.h"
#include "QueueSizes.h"
#include "CriticalSection.h"
#include "Gate.h"

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

namespace satoru {

namespace {
   typedef size_t SubscriberFlags;
   const size_t MaxSubscribersPerEventType = 8 * sizeof(SubscriberFlags);
}

class EventManager
{
public:
   template<typename T>
   static void RaiseEvent(const T& event)
   {
       static_assert(std::is_trivial<T>::value, "Event types must be trivial (default constructor and trivially copiable)");
       OperateStaticRingBuffer(AddEventOperation, &event, NULL, (T*)NULL);
   }

private:
   // Event entries carry subscriber flags to track which subscribers have already caught up
   // to the latest entry in the ringbuffer, removing the need for queue duplication.
   template<typename T>
   struct EventEntry {
      T event;
      SubscriberFlags checkedBy;
      bool beingWritten;

      EventEntry():
         checkedBy(0xFFFFFFFF),
         beingWritten(false)
      {}
   };

   template<typename T>
   struct EventRingBuffer {
      EventEntry<T> mEvent[QueueSize<T>::value];
      size_t mHead;
      size_t mTail;
      const void* mSubscriber[MaxSubscribersPerEventType];
      SubscriberFlags mSubscriberMissedEvents;

      EventRingBuffer():
         mHead(0),
         mTail(0),
         mSubscriberMissedEvents(0)
      {}

      void AddEvent(const T& event)
      {
         size_t savedTail;
         { CriticalSection section;
            savedTail = mTail;
            AdvanceTail();
            mEvent[savedTail].beingWritten = 1;
         }

         mEvent[savedTail].event = event;

         { CriticalSection section;
            mEvent[savedTail].checkedBy = 0;
            mEvent[savedTail].beingWritten = 0;
            EventManager::OperateEventGate<T>(OpenOperation);
         }
      }

      void Subscribe(const void* subscriber, bool interestedInOlderEvents)
      {
         Unsubscribe(subscriber);

         size_t subscriberIndex = 0;
         for (subscriberIndex = 0; subscriberIndex < MaxSubscribersPerEventType; subscriberIndex++) {
            if (mSubscriber[subscriberIndex] == NULL) {
               mSubscriber[subscriberIndex] = subscriber;
               break;
            }
         }

         if (!interestedInOlderEvents) {
            for (size_t eventIndex = 0; eventIndex < QueueSize<T>::value; eventIndex++) {
               { CriticalSection section;
                  mEvent[eventIndex].checkedBy |= (1 << subscriberIndex); // Flag everything as checked
               }
            }
         }
         { CriticalSection section;
            mSubscriberMissedEvents &= ~ (1 << subscriberIndex);
         }
      }

      void Unsubscribe(const void* subscriber)
      {
         size_t subscriberIndex;
         if (!GetIndexForSubscriber(subscriber, subscriberIndex)) {
            return; // Not subscribed
         }

         for (subscriberIndex = 0; subscriberIndex < MaxSubscribersPerEventType; subscriberIndex++) {
            if (mSubscriber[subscriberIndex] == subscriber) {
               mSubscriber[subscriberIndex] = NULL;
            }
         }
      }

      EventEntry<T>* GetEntry(const void* subscriber)
      {
         size_t subscriberIndex;

         if (!GetIndexForSubscriber(subscriber, subscriberIndex)) {
            return NULL;
         }

         size_t savedTail;
         size_t savedHead;

         { CriticalSection section;
            savedTail = mTail;
            savedHead = mHead;
         }

         size_t eventIndex = savedHead;
         while (eventIndex != savedTail) {
            { CriticalSection section; 
               if (!(mEvent[eventIndex].checkedBy & (1 << subscriberIndex)) && !mEvent[eventIndex].beingWritten) { 
                  mEvent[eventIndex].checkedBy |= (1 << subscriberIndex);
                  EventEntry<T>* entry =  &mEvent[eventIndex];
                  return entry;
               } else if (mEvent[eventIndex].beingWritten) {
                  return NULL;
               }
            }

            eventIndex++;
            if (eventIndex == QueueSize<T>::value) {
               eventIndex = 0;
            }
         }
         return NULL;
      }

      size_t UnreadEventsForSubscriber(const void* subscriber) const
      {
         size_t subscriberIndex;
         size_t unreadEvents = 0;
         if (!GetIndexForSubscriber(subscriber, subscriberIndex)) {
            return unreadEvents;
         }

         size_t eventIndex = mHead;
         while (eventIndex != mTail) {
            if (mEvent[eventIndex].beingWritten) {
               return unreadEvents;
            }

            if (!(mEvent[eventIndex].checkedBy & (1 << subscriberIndex))) {
               unreadEvents++;
            }

            eventIndex++;
            if (eventIndex == QueueSize<T>::value) {
               eventIndex = 0;
            }

         }

         return unreadEvents;
      }

      bool MissedEventsForSubscriber(const void* subscriber)
      {
         size_t subscriberIndex;
         if (!GetIndexForSubscriber(subscriber, subscriberIndex)) {
            return false;
         }

         if (mSubscriberMissedEvents & (1 << subscriberIndex)) {
            { CriticalSection section;
               mSubscriberMissedEvents &= ~ (1 << subscriberIndex);
            }
            return true;
         }

         return false;
      }

      bool GetIndexForSubscriber(const void* subscriber, size_t& subscriberIndex) const
      {
         for (subscriberIndex = 0; subscriberIndex < MaxSubscribersPerEventType; subscriberIndex++) {
            if (mSubscriber[subscriberIndex] == subscriber) {
               return true;
            }
         }
         return false;
      }

      void AdvanceTail()
      {
         mTail++;
         if (mTail == QueueSize<T>::value) {
            mTail = 0;
         }

         if (mHead == mTail) {
            AdvanceHead();
         }
      }

      void AdvanceHead()
      {
         // Toggles on bits for all unchecked subscribers.
         mSubscriberMissedEvents |= !mEvent[mHead].checkedBy;

         mHead++;
         if (mHead == QueueSize<T>::value) {
            mHead = 0;
         }
      }
   };

   typedef enum RingBufferOperation {
      AddEventOperation,
      GetEventOperation,
      CheckUnreadEventsOperation,
      CheckMissedEventsOperation,
      SubscribeOperation,
      SubscribeWithOldEventsOperation,
      UnsubscribeOperation
   } RingBufferOperation;

   // This method guarantees that the event has not been corrupted by another being
   // written on its place while the read was taking place.
   template<typename T>
   static bool CheckValidEventForSubscriber(const EventRingBuffer<T>& ringBuffer,
                                            const EventEntry<T>* entry, 
                                            const void* subscriber) 
   {
      size_t subscriberIndex;
      bool subscribed = ringBuffer.GetIndexForSubscriber(subscriber, subscriberIndex);
      bool valid;
      { CriticalSection section;
         valid = subscriber 
                 && subscribed 
                 && entry 
                 && !entry->beingWritten 
                 && (entry->checkedBy & (1 << subscriberIndex));
      }
      return valid;
   }

   // All access to the ringbuffers must be done in this method. This is to exploit the arbitrary
   // amounts of storage we can obtain from static variables in method scope, without requiring the
   // user to do anything to support a new event type.
   template<typename T>
   static bool OperateStaticRingBuffer(RingBufferOperation operation, const T* event, void* subscriber, T* eventToGet)
   {
      static EventRingBuffer<T> ringBuffer; // This will ensure one buffer instance per type at compile time.
      bool optionalResult = false;
      EventEntry<T>* foundEntry = NULL;

      switch (operation) {
         case AddEventOperation:
            SatoruAssert(event != NULL);
            ringBuffer.AddEvent(*event);
            break;
         case GetEventOperation:
            SatoruAssert(subscriber != NULL);
            SatoruAssert(eventToGet != NULL);
            foundEntry = ringBuffer.GetEntry(subscriber);

            if (foundEntry) {
               *eventToGet = foundEntry->event;
            }

            if (CheckValidEventForSubscriber<T>(ringBuffer, foundEntry, subscriber)) {
               optionalResult = true;
            }
            break;
         case CheckUnreadEventsOperation:
            SatoruAssert(subscriber != NULL);
            if (ringBuffer.UnreadEventsForSubscriber(subscriber) > 0) {
               optionalResult = true;
            }
            break;
         case CheckMissedEventsOperation:
            SatoruAssert(subscriber != NULL);
            optionalResult = ringBuffer.MissedEventsForSubscriber(subscriber);
            break;
         case SubscribeOperation:
            SatoruAssert(subscriber != NULL);
            ringBuffer.Subscribe(subscriber, false);
            break;
         case SubscribeWithOldEventsOperation:
            SatoruAssert(subscriber != NULL);
            ringBuffer.Subscribe(subscriber, true);
            break;
         case UnsubscribeOperation:
            SatoruAssert(subscriber != NULL);
            ringBuffer.Unsubscribe(subscriber);
            break;
         default:
            break;
      }

      return optionalResult;
   }

   typedef enum EventGateOperation {
      WaitOperation,
      OpenOperation
   } EventGateOperation;

   template<typename T>
   static void OperateEventGate(EventGateOperation operation, size_t timeout = 0)
   {
      static satoru::Gate eventGate; // One per event type.

      switch (operation)
      {
      case WaitOperation:
         if (timeout > 0) {
            eventGate.Wait(timeout);
         } else {
            eventGate.Wait();
         }
         break;
      case OpenOperation:
         eventGate.Open();
         break;
      default:
         break;
      }
   }

   EventManager() {}; // Pure static

   template<typename T>
   friend class EventConsumer;
};

}

#endif
