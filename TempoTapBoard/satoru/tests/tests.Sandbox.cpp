#include "gtest/gtest.h"

#include "../satoru/internals/Thread.h"
#include "Satoru.h"
#include "SatoruPort.h"
#include "../satoru/internals/Resources.h"

#include "../Mocks/mock.Context.h"
#include "../Mocks/mock.Critical.h"
#include "../Mocks/mock.Satoru.Port.h"

#include <cstdlib>

using namespace ::satoru;
using namespace ::std;

namespace sandbox
{

class SandboxTests : public ::testing::Test
{
public:
   static const size_t MockStackSize = 256;

   SandboxTests() {
      MockCritical_Initialise();
      MockContext_Initialise();
      MockSatoruPort::Initialise();
      // Disable the timer thread for scheduler tests
      Scheduler_TimerThread->Status = ThreadStatus_Suspended;
   }
};

TEST_F(SandboxTests, meaning_of_life)
{
   ASSERT_TRUE(true);
}

}
