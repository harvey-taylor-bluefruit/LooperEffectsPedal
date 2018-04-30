#ifndef EXAMPLE_THREAD_H
#define EXAMPLE_THREAD_H

namespace sandbox 
{

class ExampleThread
{
public:
   static void Create();

private:
   static void ThreadMain();
};

}

#endif
