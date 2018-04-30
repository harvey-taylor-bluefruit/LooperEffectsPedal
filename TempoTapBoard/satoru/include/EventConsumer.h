// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef EVENT_CONSUMER_H
#define EVENT_CONSUMER_H

#include "EventManager.h"
#include "SatoruAssert.h"
#include "Satoru.h"

namespace satoru {

template<typename T>
class EventConsumer 
{
public:
   EventConsumer(bool interestedInOldEvents = false)
   {
      if (interestedInOldEvents) {
         EventManager::OperateStaticRingBuffer<T>(EventManager::SubscribeWithOldEventsOperation, NULL, this, NULL);
      } else {
         EventManager::OperateStaticRingBuffer<T>(EventManager::SubscribeOperation, NULL, this, NULL);
      }
   }

   ~EventConsumer()
   {
      EventManager::OperateStaticRingBuffer<T>(EventManager::UnsubscribeOperation, NULL, this, NULL);
   }

   bool EventsAvailable()
   {
      return EventManager::OperateStaticRingBuffer<T>(EventManager::CheckUnreadEventsOperation, NULL, this, NULL);
   }

   bool MissedAnyEvents()
   {
      return EventManager::OperateStaticRingBuffer<T>(EventManager::CheckMissedEventsOperation, NULL, this, NULL);
   }
  
   T GetEvent()
   {
      SatoruAssert(satoru::core::IsRunning()); // Blocking method requires running scheduler
      T event;
      bool gotEvent = false;

      while (!gotEvent) {
         if (!EventsAvailable()) {
            EventManager::OperateEventGate<T>(EventManager::WaitOperation);
         }

         gotEvent = EventManager::OperateStaticRingBuffer<T>(EventManager::GetEventOperation, NULL, this, &event);
      }
      return event;
   }

   bool GetEvent(T& event, size_t timeoutMs)
   {
      SatoruAssert(satoru::core::IsRunning()); // Blocking method requires running scheduler
      if (!EventsAvailable()) {
         EventManager::OperateEventGate<T>(EventManager::WaitOperation, timeoutMs);
      }
      return EventManager::OperateStaticRingBuffer<T>(EventManager::GetEventOperation, NULL, this, &event);
   }

   void ClearEvents()
   {
      EventManager::OperateStaticRingBuffer<T>(EventManager::SubscribeOperation, NULL, this, NULL);
   }

private:
   EventConsumer(const EventConsumer&);
   const EventConsumer& operator=(const EventConsumer&);
};

}

#endif
