#include "DeviceStartup.h"
#include "Satoru.h"

#include "BlinkTest.h"
#include "ExampleThread.h"

using namespace ::sandbox;

panku devices;

int main(void)
{
   devices.Initialise();
   BlinkTest::Setup();
   ExampleThread::Create();
   satoru::core::Start();

   while (1) {
   }

   return -1; // should never reach this
}


