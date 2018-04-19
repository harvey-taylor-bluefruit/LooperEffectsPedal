#ifndef HARDWARE_STACK_FRAME_H
#define HARDWARE_STACK_FRAME_H

typedef struct {
  unsigned long r0;
  unsigned long r1;
  unsigned long r2;
  unsigned long r3;
  unsigned long r12;
  unsigned long lr;
  unsigned long pc;
  unsigned long psr;
} HardwareStackFrame;

#endif
