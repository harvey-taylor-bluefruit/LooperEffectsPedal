#pragma once

#include "../core/Critical.h"

#define assert(condition)  { \
                              if(!(condition)) { \
                                 Critical_DisallowInterrupts(); \
                                 for(;;) { \
                                 } \
                              } \
                           }

#define FATAL(message)     { \
                              (void)message; \
                              Critical_DisallowInterrupts(); \
                              for(;;) { \
                              } \
                           }
