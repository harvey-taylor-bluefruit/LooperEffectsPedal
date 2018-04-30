#ifndef MOCK_CONTEXT_H
#define MOCK_CONTEXT_H

//#include "Thread.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
using namespace ::satoru;
extern "C" {
#endif

struct MockContext {
   size_t SwitchToKernelCalls;
   size_t Switches;
   void *StackPointer;
   size_t StackSize;
   void (*EntryPoint) (void);
   void (*ExitPoint) (void);
};

extern struct MockContext mockContext;

void MockContext_Initialise(void);
bool MockContext_IsKernelThreadCurrentThread(void);

#ifdef __cplusplus
}
#endif

#endif
