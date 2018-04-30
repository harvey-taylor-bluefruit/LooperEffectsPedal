// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef DATA_ALIGN_H
#define DATA_ALIGN_H

// Support for Microsoft C++, IAR and GNU C++ data alignment

#ifdef _MSC_VER
   #define DATA_ALIGN_8(x) do { \
      __declspec(align(8))x \
   } while (0);
#elif defined(__IAR_SYSTEMS_ICC__)
   #define DATA_ALIGN_8(x) do { \
      _Pragma("data_alignment=8") \
      x \
   } while (0);
#else
   #define DATA_ALIGN_8(x) do { \
      x __attribute__ ((aligned(8))) \
   } while (0);
#endif

#endif


