##########################################
## Common visible include directories ##
##########################################

include_directories (
    #Panku
    #sandbox 
    #sandbox/Debug
    #sandbox/MedianOfFive
    #sandbox/STM32F4Drivers
    #sandbox/STM32F4Drivers/CMSIS/Device
    #sandbox/STM32F4Drivers/CMSIS/Include
    #include
)

###########################################################
## Source files common to embedded code and unit tests ##
###########################################################

set(COMMON_SOURCE
    #core/Scheduler.cpp
    #core/Timer.cpp
    #core/RecursiveMutex.cpp
    #core/CountingSemaphore.cpp
    #core/CriticalSection.cpp
    #core/Gate.cpp
    #core/Tick.cpp
    #core/StackChecks.cpp
    #core/Resources.cpp
    #core/Thread.cpp
)

#######################################################
## Source files for embedded code (STM32 specific) ##
#######################################################

set(STM32_SOURCE
    #sandbox/DeviceStartup.cpp
    # sandbox/IOPinMap.cpp
    #sandbox/main.cpp
    #sandbox/ExampleThread.cpp
    #sandbox/BlinkTest.cpp
    #sandbox/Debug/Debug.cpp
    #sandbox/Debug/TerminalDebug.cpp
    #sandbox/STM32F4Drivers/HardfaultHandler.cpp
    #sandbox/STM32F4Drivers/Clocks.cpp
    #sandbox/STM32F4Drivers/Uart.cpp
    #sandbox/STM32F4Drivers/Gpio.cpp
    #sandbox/STM32F4Drivers/SysTickTimer.cpp
    #sandbox/STM32F4Drivers/Wait.cpp
    #sandbox/SystemInit.c
    #port/stm32f4/Context.cpp
    #port/stm32f4/HardwareStackFrame.h
    #port/stm32f4/SatoruPort.cpp
    #port/stm32f4/SoftwareStackFrame.h
)

###########################################################
## Source files for embedded code (STM32-GCC specific) ##
###########################################################

set(STM32_SOURCE_GCC
    sandbox/startup_stm32f429xx_gcc.s
    port/stm32f4/Context-asm_gcc.s
    port/stm32f4/Critical-asm_gcc.s
)

###########################################################
## Source files for embedded code (STM32-IAR specific) ##
###########################################################

set(STM32_SOURCE_IAR
    sandbox/startup_stm32f429xx_IAR.s
    port/stm32f4/Context-asm_IAR.s
    port/stm32f4/Critical-asm_IAR.s
)

######################################
## Source files for unit tests only ##
######################################

set(UNIT_TEST_SOURCE
    tests/Mocks/mock.Context.c
    tests/Mocks/mock.Critical.c
    tests/Mocks/mock.Satoru.Port.cpp
)

#######################################
## Unit tests and their directories. ##
#######################################
## If you add a new whole category,  ##
## update UnitTests.cmake by adding  ##
## a build step for it.              ##
#######################################

set(LIBMOCKS
    tests/Mocks/libmock.SysTickTimer.cpp
)

set(SATORU_TESTS_DIR tests)
set(SATORU_TESTS
    Scheduler
    Ipc
    TimerThread
    Sandbox
)

