add_subdirectory(googletest)

include_directories(
    ${GTEST_INCLUDE_DIR}
    tests/NullMocks
    tests/Mocks
    core
    ipc
)

# Setup testing
enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

add_library(satoru_library ${COMMON_SOURCE} ${UNIT_TEST_SOURCE})
add_library(libmocks ${LIBMOCKS})
target_compile_definitions(satoru_library PRIVATE TESTING=1)

foreach(test ${SATORU_TESTS})
    set(TESTRUNNER_FRIENDLY_NAME ${test}Tests)
    add_executable(${TESTRUNNER_FRIENDLY_NAME} ${SATORU_TESTS_DIR}/tests.${test}.cpp)
    target_compile_definitions(${TESTRUNNER_FRIENDLY_NAME} PRIVATE TESTING=1)
 
    if (VISUAL_STUDIO_BUILD)
        target_compile_definitions(${TESTRUNNER_FRIENDLY_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS=1)
        target_link_libraries(${TESTRUNNER_FRIENDLY_NAME} satoru_library gtest gtest_main)
    else()
        target_link_libraries(${TESTRUNNER_FRIENDLY_NAME} satoru_library gtest gtest_main pthread)
    endif()
    
    add_test(${TESTRUNNER_FRIENDLY_NAME} ${TESTRUNNER_FRIENDLY_NAME})
endforeach(test)

