v1.2.1 released 19th December 2017
- Reverted one of the recent refactorings to be explicit about the word 
  size to avoid size_t evaluating to a 64 bit value in x86 cross compilations
  (caused a test to fail)
- Removed the example failing test, which was obfuscating other test failures

v1.2.0 released 18th December 2017
- Watchdog is now frozen when debug halted
- IAR warning fix
- Ash's review refactorings (including habitability improvements from Ash and Pablo)

v1.1.0 released 29th September 2017
- Separation of "sandbox" into satoru-example repository (satoru is now "pure")
- satoru is now a git submodule of satoru-example
- watchdog added (basic + multi-threaded)
- IAR port for stm32f7
- Fix semaphore with timeout of 0
- Refactored + improved debug message for handler stack check
- Safer context switch for the F7 (now follows datasheet guidelines)
