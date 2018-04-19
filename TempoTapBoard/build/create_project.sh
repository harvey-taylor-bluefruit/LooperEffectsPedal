#!/bin/bash

if [ "$1" != "" ]; then
    project_type="$1"
else
    echo "What project type do you want?" 
    echo "(1) -> Unix Makefiles (default)"
    echo "(2) -> Eclipse CDT4"

    read project_type
fi

if [ "$project_type" == "2" ]; then
    generator="Eclipse CDT4 - Unix Makefiles"
    generator_name="Eclipse"
else
    generator="Unix Makefiles"
    generator_name="Makefiles"
fi

if [ "$2" != "" ]; then
    unit_tests_choice="$2"
else
    echo "Do you want unit tests or an embedded sandbox build?" 
    echo "(1) -> Native unit tests (default)"
    echo "(2) -> Embedded code with arm-none-eabi toolchain"

    read unit_tests_choice
fi

if [ "$unit_tests_choice" == "2" ]; then 
    embedded_target_define=STM32F429xx
    which arm-none-eabi-gcc >/dev/null || { echo "arm-none-eabi-gcc must be in your PATH." >&2; exit 1; }
    folder="Embedded_$generator_name"
    mkdir $folder
    cd $folder
    cmake ../../ -DTARGET=$embedded_target_define -DCMAKE_TOOLCHAIN_FILE="../toolchain/stm32_gcc_toolchain.cmake" -G"$generator"
    cd ../
else
    folder="UnitTests_$generator_name"
    mkdir $folder
    cd $folder
    cmake ../../ -DUNIT_TESTS=true -G"$generator"
    cd ../
    echo "Remember, there is a \"make check\" command to run all tests in verbose mode!"
fi

