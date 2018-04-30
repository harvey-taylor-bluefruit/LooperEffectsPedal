#include "ExampleThread.h"

#include "Satoru.h"
#include "ThreadPriorities.h"
#include "DataAlign.h"
#include "TerminalDebug.h"
#include "SatoruDebug.h"

#include <stdio.h>

namespace sandbox
{

namespace
{
   const uint32_t aStackSize = 1024U;
   DATA_ALIGN_8(uint8_t aStack[aStackSize]);
}

void ExampleThread::ThreadMain()
{
   ::satoru::debug::Print("satoru-example ExampleThread::ThreadMain\r\n");
   while (1) {
   }
}

void ExampleThread::Create()
{
   satoru::core::CreateThread("Example",
                              aStack,
                              sizeof(aStack),
                              ExampleThread::ThreadMain,
                              ThreadPriorities::Sandbox);
}

}
