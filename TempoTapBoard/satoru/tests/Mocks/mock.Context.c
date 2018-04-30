#include "../satoru/internals/Thread.h"
#include "mock.Context.h"

#include <string.h>

struct MockContext mockContext;

void *Context_InitialiseStackFrames(void *stackBuffer,
                                    size_t stackSize,
                                    void (*entryPoint) (void),
                                    void (*exitPoint) (void))
{
   mockContext.StackPointer = (uint8_t *)stackBuffer + stackSize;
   mockContext.StackSize = stackSize;
   mockContext.EntryPoint = entryPoint;
   mockContext.ExitPoint = exitPoint;

   return mockContext.StackPointer;
}

extern Thread *Scheduler_KernelThread;
extern Thread *Scheduler_currentThread;
extern Thread *Scheduler_pendingSwitchThread;

void Context_FirstSwitchToKernelThread(void)
{
   Scheduler_currentThread = Scheduler_KernelThread;
   mockContext.SwitchToKernelCalls++;

   mockContext.StackPointer = Scheduler_currentThread->SavedStackPointer;
   mockContext.EntryPoint = Scheduler_currentThread->EntryPoint;
}

void Context_Switch(void)
{
   Scheduler_currentThread = Scheduler_pendingSwitchThread;

   mockContext.StackPointer = Scheduler_currentThread->SavedStackPointer;
   mockContext.EntryPoint = Scheduler_currentThread->EntryPoint;

   mockContext.Switches++;
}

void MockContext_Initialise(void)
{
   memset(&mockContext, 0, sizeof(mockContext));
}

bool MockContext_IsKernelThreadCurrentThread(void)
{
   return (Scheduler_KernelThread == Scheduler_currentThread);
}
