.syntax unified
.section .text

.extern Scheduler_currentThread
.extern Scheduler_pendingSwitchThread
.extern Scheduler_KernelThread

.equ FPU_USED, 0x00000010
.equ INITIAL_STACK_POINTER, _estack

.global PendSV_Handler
.thumb_func
PendSV_Handler:
   /* Disable interrupts */
   cpsid     i

   /* flush pipeline */
   isb

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
StackOverflowErrorLoop:
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
   
   /* Make sure context is ready by waiting for all memory transactions to complete and flushing pipeline */
   dsb
   isb

   /* Enable interrupts */
   cpsie   i

   bx      lr
   

.global SVC_Handler
.thumb_func
SVC_Handler:
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
   ldr     r1, =INITIAL_STACK_POINTER
   msr     msp, r1

   /* Enable interrupts */
   cpsie   i

   bx      lr /* return to thread mode */
  
.global Context_FirstSwitchToKernelThread
/* void Context_FirstSwitchToKernelThread(void) */
.thumb_func
Context_FirstSwitchToKernelThread:
   svc   0 /* triggers supervisor call with parameter 0 */
WaitForSwitch:
   bl WaitForSwitch
