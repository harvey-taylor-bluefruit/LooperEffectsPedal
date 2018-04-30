/* MIT License (https://opensource.org/licenses/MIT)
 * Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)
 */
   MODULE CONTEXT
   SECTION .text : CODE

   EXTERN Scheduler_currentThread
   EXTERN Scheduler_pendingSwitchThread
   EXTERN Scheduler_KernelThread

   ; Forward declaration of sections.
   EXTERN __ICFEDIT_intvec_start__

FPU_USED     EQU   0x00000010
INITIAL_STACK_POINTER_LOCATION   EQU __ICFEDIT_intvec_start__

   PUBLIC PendSV_Handler
   THUMB
PendSV_Handler
   /* Disable interrupts */
   cpsid     i
   /* Load process stack pointer into r0 */
   mrs   r0, psp

   /* Store FPU registers if used in previous thread */
   tst     lr, #FPU_USED
   it      eq
   vstmdbeq  r0!, {s16-s31}

   /* Store the "software stack frame" at the stack pointer
      include the Link Register so we know where to return to */
   stmfd    r0!, {r4-r11, lr}

   /* Check stack integrity */
   ldr      r4, =Scheduler_currentThread
   ldr      r1, [r4]
   ldr      r2, [r1, #4]   /* stack bottom */
   cmp      r0, r2         /* compare stack pointer with stack bottom */
StackOverflowErrorLoop
   it       mi             /* if negative, infinite loop */
   blmi     StackOverflowErrorLoop

   /* Store the current stack pointer*/
   str      r0, [r1]

   /* switch current thread */
   ldr      r3, =Scheduler_pendingSwitchThread
   ldr      r0, [r3]
   str      r0, [r4]

   /* Load new stack pointer  */
   ldr     r0, [r0]
   /* Restore registers from the new stack pointer location*/
   ldmfd   r0!, {r4-r11, lr}

   /* Restore FPU registers if previously saved */
   tst     lr, #FPU_USED
   it      eq
   vldmiaeq  r0!,      {s16-s31}
   /* Restore stack pointer */
   msr   psp, r0
   
   /* wait for memory transactions to complete to make sure stack pointer has been set */
   dsb
   isb

   /* Enable interrupts */
   cpsie   i

   bx      lr


   PUBLIC SVC_Handler
   THUMB
SVC_Handler
   /* Disable interrupts */
   cpsid   i

   /* We're not storing the old context, we're just leaping into the new one */
   /* Load new stack pointer from kernel thread pointer */
   ldr     r0, =Scheduler_KernelThread
   ldr     r1, [r0]

   /* Restore the stack pointer */
   ldr     r0, [r1]

   /* Restore registers from the new stack pointer location */
   ldmia   r0!, {r4-r11, lr}   /* load s/w stack frame */
   msr     psp, r0            /* move r0 value to psp */

   /* set msp to use linker defined stack*/
   ldr     r1, =INITIAL_STACK_POINTER_LOCATION
   ldr     r1, [r1]
   msr     msp, r1

   /* wait for memory transactions to complete to make sure stack pointers have been set */
   dsb
   isb

   /* Enable interrupts */
   cpsie   i

   bx      lr /* return to thread mode */

/* void Context_FirstSwitchToKernelThread(void) */
   PUBLIC Context_FirstSwitchToKernelThread
   THUMB
Context_FirstSwitchToKernelThread
   svc   0 /* triggers supervisor call with parameter 0 */
WaitForSwitch:
   bl WaitForSwitch

   END
