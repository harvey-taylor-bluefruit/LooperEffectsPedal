# ================================================================================================
# This is the toolchain file for  famliy using IAR compiler.
# ================================================================================================

# ------------------------------------------------------------------------------
# CMake
# ------------------------------------------------------------------------------

# Minimum CMake version.
cmake_minimum_required(VERSION 2.8)

set(IAR TRUE)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 1)

# Output
set(CMAKE_EXECUTABLE_SUFFIX ".out")

# Processor name
set(IAR_CPU_NAME Cortex-M4 CACHE STRING "Select processors.")
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)

list(APPEND CMAKE_MODULE_PATH )

set(IAR_TOOLCHAIN_NAME "ARM" CACHE INTERNAL "")
set(IAR_ARCHITECTURE "ARM" CACHE INTERNAL "")
set(IAR_CHIP_SELECTION "STM32F405VG" CACHE INTERNAL "")
set(IAR_WORKBENCH_VERSION "7.70.1.11486" CACHE INTERNAL "")
set(IAR_WORKBENCH_MAJOR_VERSION "7.4")

# Bluefruit enabled extra options
set(IAR_GEN_LOW_LEVEL_INTERFACE "FALSE" CACHE INTERNAL "")
set(IAR_CC_ENABLE_REMARKS "TRUE" CACHE INTERNAL "")
set(IAR_CC_DIAG_SUPPRESS "Pa137" CACHE INTERNAL "")
set(IAR_CC_OPT_LEVEL "1" CACHE INTERNAL "")
set(IAR_CC_OPT_LEVEL_SLAVE "1" CACHE INTERNAL "")
set(IAR_CC_DIAG_WARN_ARE_ERR "1" CACHE INTERNAL "")
set(IAR_USE_FLASH_LOADER "1" CACHE INTERNAL "")
set(IAR_DEBUGGER_CSPY_FLASHLOADER_V3 "") # TODO


set(_compiler_id_pp_test "defined(__IAR_SYSTEMS_ICC__ ) || defined(__IAR_SYSTEMS_ICC)")

# ------------------------------------------------------------------------------
# IAR directory
# ------------------------------------------------------------------------------

# This might be in PATH.
find_program(IAR_COMPILER_PATH_EXE iccarm)

if (NOT IAR_COMPILER_PATH_EXE)
message("Path to IAR Embedded Workbench ARM has not been found in PATH variable directly.")
message("Trying to find IAR Embedded Workbench ARM in Program Files...")


# 32-bit dir on win32, useless to us on win64
file(TO_CMAKE_PATH "$ENV{ProgramFiles}" PROG_FILES)

set(IAR_TOOLCHAIN_HACK_PROGFILES_X86 "ProgramFiles(x86)")

# 32-bit dir: only set on win64
file(TO_CMAKE_PATH "$ENV{${IAR_TOOLCHAIN_HACK_PROGFILES_X86}}" PROG_FILES_X86)

# 32-bit dir: only set on win64
file(TO_CMAKE_PATH "$ENV{ProgramFilesx86}" PROG_FILES_X86_GUI)

# 64-bit dir: only set on win64
file(TO_CMAKE_PATH "$ENV{ProgramW6432}" PROG_FILES_W6432)


set(IAR_ARM_PATH "IAR_ARM_PATH-NOTFOUND" CACHE PATH "Select IAR Workbench path for ARM")

if(EXISTS "${PROG_FILES}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm")
    set(IAR_ARM_PATH "${PROG_FILES}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm" CACHE PATH "" FORCE)
elseif(EXISTS "${PROG_FILES_X86}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm")
    set(IAR_ARM_PATH "${PROG_FILES_X86}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm" CACHE PATH "" FORCE)
elseif(EXISTS "${PROG_FILES_W6432}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm")
    set(IAR_ARM_PATH "${PROG_FILES_W6432}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm" CACHE PATH "" FORCE)
elseif(EXISTS "${PROG_FILES_X86_GUI}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm")
    set(IAR_ARM_PATH "${PROG_FILES_X86_GUI}/IAR Systems/Embedded Workbench ${IAR_WORKBENCH_MAJOR_VERSION}/arm" CACHE PATH "" FORCE)
endif()

if (IAR_ARM_PATH)
set (IAR_COMPILER_PATH_EXE "${IAR_ARM_PATH}/bin/iccarm.exe")

else (IAR_ARM_PATH)

message(FATAL_ERROR "IAR Embedded Workbench has not been found.")

endif (IAR_ARM_PATH)

endif(NOT IAR_COMPILER_PATH_EXE)

get_filename_component(IAR_COMPILER_PATH "${IAR_COMPILER_PATH_EXE}" DIRECTORY)
get_filename_component(IAR_ARM_PATH "${IAR_COMPILER_PATH}/.." ABSOLUTE)

set(IAR_TARGET_RTOS "none" CACHE STRING "Select target OS.")

file(GLOB IAR_ARM_PLUGIN_DIRS "${IAR_ARM_PATH}/plugins/rtos/*")
foreach(IAR_ARM_PLUGIN_DIR ${IAR_ARM_PLUGIN_DIRS})
    if(IS_DIRECTORY ${IAR_ARM_PLUGIN_DIR})

    string(REGEX MATCH "/([^/\\]+)$" DUMMY "${IAR_ARM_PLUGIN_DIR}")

    list(APPEND IAR_TARGET_RTOS_OPTIONS "${CMAKE_MATCH_1}")
    endif()
endforeach()

set_property(CACHE IAR_TARGET_RTOS PROPERTY STRINGS "${IAR_TARGET_RTOS_OPTIONS}")


# ------------------------------------------------------------------------------
# Compiler
# ------------------------------------------------------------------------------

# Set compilers flags

set(IAR_COMPILER_DLIB_CONFIG "Normal" CACHE STRING "Select DLIB config (default: Normal).")
set(IAR_DLIB_CONFIG_OPTIONS "Normal" "Full")
set_property(CACHE IAR_COMPILER_DLIB_CONFIG PROPERTY STRINGS ${IAR_DLIB_CONFIG_OPTIONS})

set(DLIB_HEADER_PATH \"${IAR_ARM_PATH}/inc/c/DLib_Config_${IAR_COMPILER_DLIB_CONFIG}.h\")

set(IAR_COMPILER_FLAGS
    "-e"
    "--diag_suppress Pa050"
    "--cpu_mode thumb"
    "--endian=little"
    "--cpu=${IAR_CPU_NAME}"
    "--dlib_config ${DLIB_HEADER_PATH}"
)

set(IAR_COMPILER_FLAGS_CXX
    ${IAR_COMPILER_FLAGS}
    "--c++"
    "--no_exceptions"
    "--no_rtti"
    "--no_static_destruction"
)

set(IAR_COMPILER_FLAGS_C
    ${IAR_COMPILER_FLAGS}
    "--use_c++_inline"
)

string(REPLACE ";" " " IAR_COMPILER_FLAGS_C_STR "${IAR_COMPILER_FLAGS_C}")
string(REPLACE ";" " " IAR_COMPILER_FLAGS_CXX_STR "${IAR_COMPILER_FLAGS_CXX}")

set(CMAKE_CXX_FLAGS "${IAR_COMPILER_FLAGS_CXX_STR}" CACHE STRING "")
set(CMAKE_C_FLAGS "${IAR_COMPILER_FLAGS_C_STR}" CACHE STRING "")

set(CMAKE_C_FLAGS_DEBUG "--debug -On --guard_calls" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "--debug -On --guard_calls" CACHE STRING "")

set(CMAKE_C_FLAGS_RELEASE "-Ohz" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-Ohz" CACHE STRING "")

set(CMAKE_ASM_IAR_FLAGS "-s+ -M\"<>\" -w+ -r --cpu Cortex-R4 --fpu VFPv3_D16" CACHE STRING "")

set(CMAKE_C_COMPILER_ID "IAR")
set(CMAKE_CXX_COMPILER_ID "IAR")
set(CMAKE_C_COMPILER "${IAR_COMPILER_PATH_EXE}" CACHE FILEPATH "C Compiler" )
set(CMAKE_CXX_COMPILER "${IAR_COMPILER_PATH_EXE}" CACHE FILEPATH "C++ Compiler")
set(CMAKE_LINKER "${IAR_COMPILER_PATH}/ilinkarm.exe" CACHE FILEPATH "IAR Linker")
set(CMAKE_AR "${IAR_COMPILER_PATH}/iarchive.exe" CACHE FILEPATH "IAR Archiver")
set(CMAKE_ASM_COMPILER "${IAR_COMPILER_PATH}/iasmarm.exe" CACHE FILEPATH "ASM Compiler" )
set(CMAKE_ASM_IAR_COMPILER "${IAR_COMPILER_PATH}/iasmarm.exe" CACHE FILEPATH "ASM Compiler" )

set(CMAKE_INCLUDE_FLAG_CXX "-I")
set(CMAKE_CXX_COMPILE_OBJECT  "<CMAKE_CXX_COMPILER> <FLAGS> <DEFINES> <INCLUDES> <SOURCE> -o <OBJECT>")
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> <FLAGS> <DEFINES> <INCLUDES> <SOURCE> -o <OBJECT>")


set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> <TARGET> --create <LINK_FLAGS> <OBJECTS> ")

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> <TARGET> --create <LINK_FLAGS> <OBJECTS> ")


# ------------------------------------------------------------------------------
# Linker
# ------------------------------------------------------------------------------

set(IAR_LINKER_ICF_FILE "${CMAKE_SOURCE_DIR}/build/toolchain/LinkerScript_IAR.icf" CACHE FILEPATH ".icf Linker file for ILINK.")

set(IAR_LINKER_ENTRY_ROUTINE "__iar_program_start" CACHE STRING "IAR Linker entry point routine.")

if (NOT IAR_LINKER_ICF_FILE STREQUAL "")

set(IAR_LINKER_ICF_OVERRIDE "1" CACHE INTERNAL "")

set( IAR_LINKER_FLAGS
    "--redirect _Printf=_PrintfSmallNoMb"
    "--redirect _Scanf=_ScanfSmallNoMb"
    "--redirect __write=__write_buffered"
    "--map ${PROJECT_NAME}.map"
    "--config ${IAR_LINKER_ICF_FILE}"
    "--entry ${IAR_LINKER_ENTRY_ROUTINE}"
    "--inline"
    "--no_exceptions"
    "--merge_duplicate_sections"
    "--vfe"
)

string(REPLACE ";" " " IAR_LINKER_FLAGS_STR "${IAR_LINKER_FLAGS}")

# Is this the absolute minimum set of options necessary for linking?
set(CMAKE_EXE_LINKER_FLAGS "${IAR_LINKER_FLAGS_STR}"
    CACHE STRING "LINKER FLAGS" FORCE
)

endif()

# ------------------------------------------------------------------------------
# CMAKE Environment & Paths
# ------------------------------------------------------------------------------

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH "${IAR_ARM_PATH}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# ------------------------------------------------------------------------------
# IAR Debugger
# ------------------------------------------------------------------------------

set(IAR_DEBUGGER_CSPY_MEMFILE "" CACHE FILEPATH "C-Spy Mem file path.")
set(IAR_DEBUGGER_CSPY_MACFILE "" CACHE FILEPATH "C-Spy MAC file path.")
set(IAR_DEBUGGER_CSPY_EXTRAOPTIONS "--jet_use_hw_breakpoint_for_semihosting" CACHE STRING "C-Spy debugger extra options")
set(IAR_DEBUGGER_CSPY_FLASHLOADER_V3 "" CACHE FILEPATH "Flashloader v3 file path.")

set(IAR_DEBUGGER_IJET_PROBECONFIG "" CACHE FILEPATH "Probe configuration file for I-Jet.")

set(IAR_DEBUGGER_LOGFILE "" CACHE FILEPATH "General log file path.")

# ------------------------------------------------------------------------------
# Checks.
# ------------------------------------------------------------------------------

# -e            Standard with IAR extensions
# none          Standard
# -c89          C89 (Default is C99)

# Summary of compiler options. See doc:
# http://supp.iar.com/FilesPublic/UPDINFO/005832/arm/doc/EWARM_DevelopmentGuide.ENU.pdf

