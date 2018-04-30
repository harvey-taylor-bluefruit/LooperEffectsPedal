// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *Context_InitialiseStackFrames(void *stackPointer, 
                                   size_t stackSize,
                                   void (*entryPoint) (void),
                                   void (*exitPoint) (void));
void Context_FirstSwitchToKernelThread(void);
void Context_Switch(void);

#ifdef __cplusplus
}
#endif


#endif
