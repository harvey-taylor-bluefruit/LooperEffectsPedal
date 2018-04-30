if (NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "You must specify a toolchain file! Use the -DCMAKE_TOOLCHAIN_FILE option.")
endif()

if (IAR)
    add_executable(sandbox ${COMMON_SOURCE} ${STM32_SOURCE} ${STM32_SOURCE_IAR})
else()
    add_executable(sandbox ${COMMON_SOURCE} ${STM32_SOURCE} ${STM32_SOURCE_GCC})
    set(CHAIN arm-none-eabi-)
endif()

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/build/toolchain/${LINKER_SCRIPT})
target_compile_definitions(sandbox PRIVATE ${TARGET} ${MCU_FAMILY} ECB __STDC_LIMIT_MACROS SATORU_DEBUG)
add_custom_target(connect_jlink COMMAND openocd -f ${CMAKE_SOURCE_DIR}/build/toolchain/stm32_jlink.cfg &)
add_custom_target(connect_stlinkV2 COMMAND openocd -f ${CMAKE_SOURCE_DIR}/build/toolchain/stm32_stlinkV2.cfg &)


if ("${TARGET}" STREQUAL "STM32F746xx")
   add_custom_target(connect_stlinkV2-1 COMMAND openocd -f ${CMAKE_SOURCE_DIR}/build/toolchain/stm32_stlinkV2-1_stm32f746.cfg &)
else()
   add_custom_target(connect_stlinkV2-1 COMMAND openocd -f ${CMAKE_SOURCE_DIR}/build/toolchain/stm32_stlinkV2-1_stm32f429.cfg &)
endif()

add_custom_target(debug COMMAND ${CHAIN}gdb-py -x ${CMAKE_SOURCE_DIR}/build/toolchain/GdbScript.txt)
