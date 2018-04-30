##########################################
## Common visible include directories ##
##########################################

include_directories (
    panku
    sandbox 
    sandbox/debug
    satoru/include
)

if ("${TARGET}" STREQUAL "STM32F746xx")
   include_directories (
       sandbox/stm32f7_drivers
       sandbox/stm32f7_drivers/cmsis/device
       sandbox/stm32f7_drivers/cmsis/include
   )
else()
   include_directories (
       sandbox/stm32f4_drivers
       sandbox/stm32f4_drivers/cmsis/device
       sandbox/stm32f4_drivers/cmsis/include
   )
endif()

###########################################################
## Source files common to embedded code and unit satoru/tests ##
###########################################################

set(COMMON_SOURCE
    satoru/internals/Satoru.cpp
    satoru/internals/Timer.cpp
    satoru/internals/RecursiveMutex.cpp
    satoru/internals/CountingSemaphore.cpp
    satoru/internals/CriticalSection.cpp
    satoru/internals/Gate.cpp
    satoru/internals/Tick.cpp
    satoru/internals/StackChecks.cpp
    satoru/internals/Resources.cpp
    satoru/internals/Thread.cpp
)

#######################################################
## Source files for embedded code (STM32 specific) ##
#######################################################

set(STM32_SOURCE_GENERAL
    sandbox/DeviceStartup.cpp
    sandbox/IOPinMap.cpp
    sandbox/main.cpp
    sandbox/ExampleThread.cpp
    sandbox/BlinkTest.cpp
    sandbox/debug/Debug.cpp
    sandbox/debug/TerminalDebug.cpp
    sandbox/SystemInit.c
    satoru/port/stm32/Context.cpp
)

set(STM32F4_SOURCE
    sandbox/stm32f4_drivers/HardfaultHandler.cpp
    sandbox/stm32f4_drivers/Clocks.cpp
    sandbox/stm32f4_drivers/Uart.cpp
    sandbox/stm32f4_drivers/Gpio.cpp
    sandbox/stm32f4_drivers/SysTickTimer.cpp
    sandbox/stm32f4_drivers/Wait.cpp
    satoru/port/stm32/stm32f4/SatoruPort.cpp
)

set(STM32F7_SOURCE
    sandbox/stm32f7_drivers/HardfaultHandler.cpp
    sandbox/stm32f7_drivers/Clocks.cpp
    sandbox/stm32f7_drivers/Uart.cpp
    sandbox/stm32f7_drivers/Gpio.cpp
    sandbox/stm32f7_drivers/SysTickTimer.cpp
    sandbox/stm32f7_drivers/Wait.cpp
    satoru/port/stm32/stm32f7/SatoruPort.cpp
)

if ("${TARGET}" STREQUAL "STM32F746xx")
   set(STM32_SOURCE ${STM32_SOURCE_GENERAL} ${STM32F7_SOURCE})
   set(STM32_PORT_FOLDER stm32f7)
else()
   set(STM32_SOURCE ${STM32_SOURCE_GENERAL} ${STM32F4_SOURCE})
   set(STM32_PORT_FOLDER stm32f4)
endif()

###########################################################
## Source files for embedded code (STM32-GCC specific) ##
###########################################################
if ("${TARGET}" STREQUAL "STM32F746xx")
   set(STM32_SOURCE_GCC
       sandbox/startup_stm32f746xx_gcc.s
   )
else()
   set(STM32_SOURCE_GCC
       sandbox/startup_stm32f429xx_gcc.s
   )
endif()

set(STM32_SOURCE_GCC
    ${STM32_SOURCE_GCC}
    satoru/port/stm32/${STM32_PORT_FOLDER}/Context-asm_gcc.s
    satoru/port/stm32/Critical-asm_gcc.s
)

###########################################################
## Source files for embedded code (STM32-IAR specific) ##
###########################################################

if ("${TARGET}" STREQUAL "STM32F746xx")
   set(STM32_SOURCE_IAR
       sandbox/startup_stm32f746xx_IAR.s
   )
else()
   set(STM32_SOURCE_IAR
       sandbox/startup_stm32f429xx_IAR.s
   )
endif()

set(STM32_SOURCE_IAR
   ${STM32_SOURCE_IAR}
   satoru/port/stm32/${STM32_PORT_FOLDER}/Context-asm_IAR.s
   satoru/port/stm32/Critical-asm_IAR.s
)

######################################
## Source files for unit satoru/tests only ##
######################################

set(UNIT_TEST_SOURCE
    satoru/tests/Mocks/mock.Context.c
    satoru/tests/Mocks/mock.Critical.c
    satoru/tests/Mocks/mock.Satoru.Port.cpp
)

#######################################
## Unit satoru/tests and their directories. ##
#######################################
## If you add a new whole category,  ##
## update UnitTests.cmake by adding  ##
## a build step for it.              ##
#######################################

set(LIBMOCKS
    satoru/tests/Mocks/libmock.SysTickTimer.cpp
)

set(SATORU_TESTS_DIR satoru/tests)
set(SATORU_TESTS
    Scheduler
    Ipc
    TimerThread
    Sandbox
)
