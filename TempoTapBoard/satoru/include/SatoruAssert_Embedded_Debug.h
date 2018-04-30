// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SCHEDULER_ASSERT_EMBEDDED_DEBUG_H
#define SCHEDULER_ASSERT_EMBEDDED_DEBUG_H

#include "../internals/Critical.h"
#include "SatoruDebug.h"

#define LINE_AS_STRING(line)  #line
#define LINE_NUMBER(line)  LINE_AS_STRING(line)

#define FATAL(x) do { \
   ASSERT_PRINT(x); \
   ASSERT_PRINT("\r\n*** FATAL ERROR in " __FILE__ ", Line " \
      LINE_NUMBER(__LINE__) "\r\n*** Function: "); \
   ASSERT_PRINT(__func__); \
   ASSERT_PRINT("\r\n*** PROGRAM HALTED! ***"); \
   Critical_DisallowInterrupts(); \
   while (1) { \
   } \
} while (0);

#define assert(condition)  do { \
   if(!(condition)) { \
      ASSERT_PRINT("\r\n*** ASSERT FAILED: \"" #condition "\" in " __FILE__ ", Line " \
         LINE_NUMBER(__LINE__) "\r\n*** Function: "); \
      ASSERT_PRINT(__func__); \
      ASSERT_PRINT("\r\n*** PROGRAM HALTED! ***"); \
      Critical_DisallowInterrupts(); \
      while (1) { \
      } \
   } \
} while (0);

#endif

