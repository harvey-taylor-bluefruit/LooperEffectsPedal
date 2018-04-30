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
    generator_name="makefiles"
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
    echo "What target?"
    echo "(note - sandbox is set up for a Nucleo 144 with STM32F429)"
    echo "(1) STM32F405"
    echo "(2) STM32F429 (default)" 
    echo "(3) STM32F746" 
    read embedded_target

    embedded_target_define=STM32F429xx
    linker_script="LinkerScript_stm32f429_gcc.ld"

    if [ "$embedded_target" == "1" ]; then
        embedded_target_define=STM32F405xx
        linker_script="LinkerScript_stm32f405_gcc.ld"
    elif [ "$embedded_target" == "3" ]; then
        embedded_target_define=STM32F746xx
        linker_script="LinkerScript_stm32f746_gcc.ld"
    fi

    which arm-none-eabi-gcc >/dev/null || { echo "arm-none-eabi-gcc must be in your PATH." >&2; exit 1; }
    folder="embedded_$embedded_target_define"_$generator_name
    mkdir $folder
    cp helper_scripts/* $folder/
    cd $folder
    makefile_dir="$PWD"
    cmake ../../ -DTARGET=$embedded_target_define -DCMAKE_TOOLCHAIN_FILE="../toolchain/stm32_gcc_toolchain.cmake" -DLINKER_SCRIPT=$makefile_dir/../toolchain/$linker_script -G"$generator"
    cd ../
else
    folder="unit_tests_$generator_name"
    mkdir $folder
    cd $folder
    cmake ../../ -DUNIT_TESTS=true -G"$generator"
    cd ../
    echo "Remember, there is a \"make check\" command to run all tests in verbose mode!"
fi

