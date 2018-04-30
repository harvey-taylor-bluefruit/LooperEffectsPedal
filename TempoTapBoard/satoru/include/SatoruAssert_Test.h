// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#pragma once

#include <stdexcept>
#include <iostream>

#define LINE_AS_STRING(line)  #line
#define LINE_NUMBER(line)  LINE_AS_STRING(line)

#ifndef assert
   #define assert(condition) do { \
      if(!(condition)) { \
         throw \
            std::runtime_error("ASSERT FAILED: \"" #condition "\" in " __FILE__ ", Line " LINE_NUMBER(__LINE__)); \
      } \
   } while (0);
#endif

#define FATAL(x) do { \
   std::cout << x << std::flush; \
   throw std::runtime_error("FATAL ERROR: in " __FILE__ ", Line " LINE_NUMBER(__LINE__)); \
} while (0);

#define UNUSED(x) (void)(x)


