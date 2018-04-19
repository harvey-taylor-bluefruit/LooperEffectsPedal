cmake_minimum_required(VERSION 3.4)
project(sandbox)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 14)

if (NOT NO_GNU_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -g")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MTd")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
endif ()

include(build/Sources.cmake)

if(UNIT_TESTS)
    include(build/UnitTests.cmake)
else()
    include(build/Embedded.cmake)
endif()
