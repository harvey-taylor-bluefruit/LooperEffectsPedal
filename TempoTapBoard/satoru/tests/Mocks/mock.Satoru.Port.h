#ifndef MOCK_SCHEDULER_PORT_H
#define MOCK_SCHEDULER_PORT_H

#include <stdint.h>
#include <stddef.h>

namespace MockSatoruPort
{
   void Initialise();
   extern size_t portInitCalls;
}

#endif
