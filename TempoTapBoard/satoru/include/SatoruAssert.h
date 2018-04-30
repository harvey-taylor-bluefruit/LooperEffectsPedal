// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef SATORU_ASSERT_H
#define SATORU_ASSERT_H

#if !defined(TESTING) 
   #if !defined(NDEBUG)
      #ifdef __cplusplus
         #define ASSERT_PRINT(message)  { satoru::debug::Print(message); }
      #else
         #define ASSERT_PRINT(message)  { SatoruDebug_Print(message); }
      #endif
      #include "SatoruAssert_Embedded_Debug.h"
   #else
      #include "SatoruAssert_Embedded_Release.h"
   #endif
#else
   #include "SatoruAssert_Test.h"
#endif

#define SatoruAssert assert

#endif


