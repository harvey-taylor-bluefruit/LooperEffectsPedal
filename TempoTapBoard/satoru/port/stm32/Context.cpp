// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "../../internals/Context.h"
#include "HardwareStackFrame.h"
#include "SoftwareStackFrame.h"
#include "../../internals/Thread.h"
#ifdef STM32F7
#include "stm32f7xx.h"
#else
#include "stm32f4xx.h"
#endif

#include <SatoruAssert.h>
#include <string.h>

using namespace ::satoru;

extern "C" Thread *Scheduler_currentThread;

void Context_Switch(void)
{
   SatoruAssert(Scheduler_currentThread);

   SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; // triggers PendSV interrupt: the handler runs in privileged mode and will perform the actual switch
}

/* Hardware stack frame :
 *
 *                     <previous>    <--- SP here before interrupt
 * SP + 0x1C              xPSR
 * SP + 0x18               PC
 * SP + 0x14               LR
 * SP + 0x10               R12
 * SP + 0x0C               R3
 * SP + 0x08               R2
 * SP + 0x04               R1
 * SP + 0x00               R0        <--- SP here after interrupt
 *
 */

static const size_t InitialExcReturnValue = 0xFFFFFFFDU; // Return to Thread mode, exception return uses non-floating-point state and execution uses PSP after return.

void *Context_InitialiseStackFrames(void *stackBuffer,
                                    size_t stackSize,
                                    void (*entryPoint) (void),
                                    void (*exitPoint) (void))
{
   SatoruAssert(stackSize > (sizeof(HardwareStackFrame) + sizeof(SoftwareStackFrame)));
   // double word alignment, to conform with Procedure Call Standard for the ARM� Architecture (AAPCS)
   SatoruAssert((reinterpret_cast<unsigned long>(stackBuffer) & 0x00000007) == 0);
   SatoruAssert((stackSize % 8) == 0);

   const unsigned long SoftwareStackSize = sizeof(SoftwareStackFrame);
   const unsigned long HardwareStackSize = sizeof(HardwareStackFrame);

   const unsigned long HardwareStackOffset = stackSize - HardwareStackSize;
   const unsigned long SoftwareStackOffset = HardwareStackOffset - SoftwareStackSize;

   unsigned long *HardwareFrameLocation =
      reinterpret_cast<unsigned long*> (
         reinterpret_cast<unsigned long> (stackBuffer) + HardwareStackOffset);
   HardwareStackFrame *hardwareFrame = reinterpret_cast<HardwareStackFrame *>(HardwareFrameLocation);
   hardwareFrame->r0 = 0;
   hardwareFrame->r1 = 1;
   hardwareFrame->r2 = 2;
   hardwareFrame->r3 = 3;
   hardwareFrame->r12 = 12;
   hardwareFrame->pc = reinterpret_cast<unsigned long>(entryPoint);
   hardwareFrame->lr = reinterpret_cast<unsigned long>(exitPoint);
   hardwareFrame->psr = 0x01000000; // set Thumb state bit

   unsigned long *SoftwareFrameLocation =
      reinterpret_cast<unsigned long*> (
         reinterpret_cast<unsigned long> (stackBuffer) + SoftwareStackOffset);
   SoftwareStackFrame *softwareFrame =
      reinterpret_cast<SoftwareStackFrame*> (SoftwareFrameLocation);

   softwareFrame->r4 = 4;
   softwareFrame->r5 = 5;
   softwareFrame->r6 = 6;
   softwareFrame->r7 = 7;
   softwareFrame->r8 = 8;
   softwareFrame->r9 = 9;
   softwareFrame->r10 = 10;
   softwareFrame->r11 = 11;

   softwareFrame->exc_return = InitialExcReturnValue;

   return SoftwareFrameLocation;
}

