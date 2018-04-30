#ifdef STM32F7
#include "stm32f7xx.h"
#else
#include "stm32f4xx.h"
#endif
#include "TerminalDebug.h"
#include "SatoruDebug.h"

#if defined(SATORU_DEBUG) && !defined(NDEBUG)

void satoru::debug::Print(const char *message)
{
#ifndef TERMINAL_DEBUG
   while(*message != '\0') {
      ITM_SendChar(*message);
      message++;
   }
#else
   if (sandbox::terminalUart != nullptr) {
      sandbox::terminalUart->SendStringBlocking(message);
   }   
#endif
}

extern "C" void SatoruDebug_Print(const char *message)
{
   satoru::debug::Print(message);
}

#endif
