#ifndef MOCK_CRITICAL_H
#define MOCK_CRITICAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOCK_CRITICAL_MAX_HISTORY 100

struct MockCriticalHistory {
   size_t EntryCount;
   bool History[MOCK_CRITICAL_MAX_HISTORY];
   bool LimitReached;
};

extern struct MockCriticalHistory mockCriticalHistory;

void MockCritical_Initialise(void);

#ifdef __cplusplus
}
#endif

#endif
