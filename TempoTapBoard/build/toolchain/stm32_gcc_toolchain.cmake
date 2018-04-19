# Targeting an embedded system, no OS.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR "cortex-m4")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(ASM_OPTIONS "-x assembler-with-cpp --gstabs+")
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

set(COMMON_FLAGS  "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(COMPILE_FLAGS "${COMMON_FLAGS} -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -fno-exceptions -MMD -MP")
set(LINK_FLAGS "-Wl,-Map=output.map -Wl,--gc-sections -fno-exceptions -fno-rtti  -specs nosys.specs -specs nano.specs -T ${CMAKE_SOURCE_DIR}/build/toolchain/LinkerScript_gcc.ld")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} -fno-rtti" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}" CACHE STRING "" FORCE)

set(BUILD_SHARED_LIBS OFF)
