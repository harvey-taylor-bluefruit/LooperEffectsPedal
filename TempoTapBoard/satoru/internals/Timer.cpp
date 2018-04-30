// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "Timer.h"
#include "Satoru.h"
#include "CountingSemaphore.h"
#include "SatoruPort.h"
#include "Resources.h"
#include "SatoruAssert.h"
#include "CriticalSection.h"
#include "../internals/TimerEntry.h"

#include <stdint.h>

namespace satoru {

static bool Earlier(const TimerEntry& entryA, const TimerEntry& entryB)
{
   return (entryA.absoluteTimeTicks - resources.tickCounter) < (entryB.absoluteTimeTicks - resources.tickCounter);
}

static void InsertBetween(TimerEntry* entryToInsert, TimerEntry* previousEntry, TimerEntry* nextEntry)
{
   if (previousEntry) {
      previousEntry->next = entryToInsert;
      entryToInsert->previous = previousEntry;
   } else if (resources.firstPendingTimer && !resources.firstActiveTimer) {
      // Edge case: there is a chain of timers at resources.firstPendingTimer, but no active
      // (ticking down) timers. In this case, we must append the new entry at the 
      // end of the chain.
      TimerEntry* entryIterator = resources.firstPendingTimer;
      while (entryIterator->next) {
         entryIterator = entryIterator->next;
      }
      entryIterator->next = entryToInsert;
      entryToInsert->previous = entryIterator;
   }

   if (nextEntry) {
      nextEntry->previous = entryToInsert;
      entryToInsert->next = nextEntry;
   }

   if ((nextEntry && (nextEntry == resources.firstPendingTimer))
       || !resources.firstPendingTimer) 
   {
      resources.firstPendingTimer = entryToInsert;
   }

   if ((nextEntry && (nextEntry == resources.firstActiveTimer))
       || !resources.firstActiveTimer) 
   {
      resources.firstActiveTimer = entryToInsert;
   }

}

static void ScheduleEntry(TimerEntry* entry, size_t absoluteTimeTicks) 
{
   CriticalSection section;
   SatoruAssert(entry->next == NULL)
   SatoruAssert(entry->previous == NULL)

   entry->absoluteTimeTicks = absoluteTimeTicks;

   if (!resources.firstActiveTimer) {
      InsertBetween(entry, NULL, NULL);
   } else if (Earlier(*entry, *resources.firstActiveTimer)) {
      InsertBetween(entry, NULL, resources.firstActiveTimer);
   } else {
      TimerEntry* entryIterator = resources.firstActiveTimer;
      while (entryIterator->next) {
         if (Earlier(*entry, *(entryIterator->next))) {
            InsertBetween(entry, entryIterator, entryIterator->next);
            return;
         }
         entryIterator = entryIterator->next;
      }

      // We reached the end.
      InsertBetween(entry, entryIterator, NULL);
   }

   SatoruAssert(entry->next != entry);
   SatoruAssert(entry->previous != entry);
}

void ServiceTimerList() 
{
   resources.timerThreadSemaphore.Take();
   while (resources.firstPendingTimer && (resources.firstPendingTimer != resources.firstActiveTimer)) {
      TimerEntry* entryToFire;
      { CriticalSection section;
         entryToFire = resources.firstPendingTimer;

         // Cuts the entry out of the chain.
         resources.firstPendingTimer = entryToFire->next;
         if (entryToFire->next) {
            entryToFire->next->previous = NULL;
            entryToFire->next = NULL;
         }
         entryToFire->previous = NULL;

         // Re-schedules into the chain if the timer is repeating
         if (entryToFire->repeatPeriod > 0) {
            ScheduleEntry(entryToFire, entryToFire->absoluteTimeTicks 
                                       + entryToFire->repeatPeriod);
         }
      }

      entryToFire->callback();
   }
}

Timer::Timer(Timer::Callback callback, size_t timeMs, bool repeating):
   mEntry(NULL),
   mTimeTicks(port::MsToTicks(timeMs))
{
   SatoruAssert(callback != NULL);
   size_t slot = UINT32_MAX;
   for (size_t index = 0; index < config::MaximumActiveTimers; index++) {
      if (!resources.timerList[index].callback) {
         slot = index;
         break;
      }
   }
   SatoruAssert(slot < config::MaximumActiveTimers);
   mEntry = &resources.timerList[slot];
   mEntry->callback = callback;

   if (repeating) {
      mEntry->repeatPeriod = port::MsToTicks(timeMs);
   }
}

Timer::~Timer()
{
   Stop();
   *mEntry = TimerEntry();
}

void Timer::Reconfigure(Timer::Callback callback, size_t timeMs, bool repeating)
{
   Stop();
   mEntry->callback = callback;
   if (repeating) {
      mEntry->repeatPeriod = port::MsToTicks(timeMs);
   }
   mTimeTicks = port::MsToTicks(timeMs);
}

void Timer::Start() 
{
   Stop();
   size_t absoluteTimeTicks = resources.tickCounter + mTimeTicks;
   ScheduleEntry(mEntry, absoluteTimeTicks);
}

void Timer::Stop() 
{
   CriticalSection section;
   // Cut the timer out of the timer chain.
   if (mEntry == resources.firstPendingTimer) {
      resources.firstPendingTimer = mEntry->next;
   }
   if (mEntry == resources.firstActiveTimer) {
      resources.firstActiveTimer = mEntry->next;
   }
   if (mEntry->previous) {
      mEntry->previous->next = mEntry->next;
   }
   if (mEntry->next) {
      mEntry->next->previous = mEntry->previous;
   }
   mEntry->next = NULL;
   mEntry->previous = NULL;
}

bool Timer::IsRunning()
{
   TimerEntry* entryIterator = resources.firstPendingTimer;
   while (entryIterator) {
      if (entryIterator == mEntry) {
         return true;
      }
      entryIterator = entryIterator->next;
   }

   return false;
}

}
