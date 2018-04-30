// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

namespace satoru
{

// RAII wrapper for port defined critical section
class CriticalSection 
{
public:
   CriticalSection();
   ~CriticalSection();
private:
   CriticalSection(const CriticalSection&);
   const CriticalSection& operator=(const CriticalSection&);
};

}

#endif
