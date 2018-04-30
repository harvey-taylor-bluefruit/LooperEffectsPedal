#include "mock.Satoru.Port.h"
#include "SatoruPort.h"
#include "../satoru/internals/Resources.h"

void satoru::port::Initialise()
{
   MockSatoruPort::portInitCalls++;
}

size_t satoru::port::MsToTicks(size_t ms)
{
   return ms;
}

size_t MockSatoruPort::portInitCalls;

void MockSatoruPort::Initialise()
{
   portInitCalls = 0;
   // Resetting here to clear state between tests
   satoru::resources.Reset();
}
