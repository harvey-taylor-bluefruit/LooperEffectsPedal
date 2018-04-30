#include "mock.Critical.h"
#include "../satoru/internals/Critical.h"

#include <string.h>

struct MockCriticalHistory mockCriticalHistory;

void Critical_AllowInterrupts(void)
{
   if (mockCriticalHistory.EntryCount == MOCK_CRITICAL_MAX_HISTORY)
   {
      mockCriticalHistory.LimitReached = true;
      return;
   }

   mockCriticalHistory.History[mockCriticalHistory.EntryCount] = true;
   mockCriticalHistory.EntryCount++;
}

void Critical_DisallowInterrupts(void)
{
   if (mockCriticalHistory.EntryCount == MOCK_CRITICAL_MAX_HISTORY)
   {
      mockCriticalHistory.LimitReached = true;
      return;
   }

   mockCriticalHistory.History[mockCriticalHistory.EntryCount] = false;
   mockCriticalHistory.EntryCount++;
}

void MockCritical_Initialise(void)
{
   memset(&mockCriticalHistory, 0, sizeof(mockCriticalHistory));
}
