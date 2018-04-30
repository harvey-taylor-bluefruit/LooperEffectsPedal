#ifndef DATA_ALIGN_H
#define DATA_ALIGN_H

// Support for IAR and GNU C++ data alignment

#ifdef __IAR_SYSTEMS_CC__
   #define DATA_ALIGN_8(x) \
      #pragma data_alignment = 8 \
      x;
#else
   #define DATA_ALIGN_8(x) \
      x __attribute__ ((aligned(8)));
#endif

#endif

