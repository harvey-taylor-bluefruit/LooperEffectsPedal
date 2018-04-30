#ifndef GATE_H
#define GATE_H

#include <stdint.h>
#include <stddef.h>

namespace satoru 
{

// Any number of threads may block (Wait at the gate), and they are all
// released at once by a single Open().
class Gate 
{
public:
   void Wait(); 
   bool Wait(size_t timeoutMs); // Returns false on timeout.
   void Open(); // Frees all blocked threads.

   Gate();
   ~Gate();

private:
   Gate(const Gate&);
   const Gate& operator=(const Gate&);
};

}

#endif

