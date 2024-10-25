cmake_minimum_required(VERSION 3.19)
project(dsa)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

set(ALGOS_DIR ${CMAKE_SOURCE_DIR}/algos)
set(TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

set(ALGOS_BIN_DIR ${CMAKE_BINARY_DIR}/algos)
set(TESTS_BIN_DIR ${CMAKE_BINARY_DIR}/tests)

file(GLOB ALGO_SOURCES "${ALGOS_DIR}/*.cpp")
file(GLOB TEST_SOURCES "${TESTS_DIR}/*.cpp")

# Build algo binaries
foreach(ALGO_FILE ${ALGO_SOURCES})
    get_filename_component(ALGO ${ALGO_FILE} NAME_WE)

    MESSAGE([ALGO] " Adding algo executable: algos/${ALGO}")

    add_executable(${ALGO} ${ALGO_FILE})
    set_property(TARGET ${ALGO} PROPERTY COMPILE_WARNING_AS_ERROR ON)

    target_compile_options(${ALGO} PRIVATE
        -Wall
        -Wextra
        -Wsign-compare
        -pedantic
        -Werror
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
    )

    target_link_options(${ALGO} PRIVATE
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
    )

    set_target_properties(${ALGO} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ALGOS_BIN_DIR}
    )
endforeach()

add_subdirectory(fuzztest)
enable_testing()
include(GoogleTest)

# Build test binaries
foreach(ALGO_FILE ${ALGO_SOURCES})
    get_filename_component(ALGO ${ALGO_FILE} NAME_WE)

    set(TEST_FILE "${TESTS_DIR}/${ALGO}.cpp")
    if(EXISTS ${TEST_FILE})
        MESSAGE([TEST] " Adding test executable: tests/${ALGO}")

        fuzztest_setup_fuzzing_flags()

        add_executable(test_${ALGO} ${TEST_FILE} ${ALGO_FILE})
        target_compile_definitions(test_${ALGO} PRIVATE NOMAIN)

        set_target_properties(test_${ALGO} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${TESTS_BIN_DIR}
            OUTPUT_NAME ${ALGO}
        )

        link_fuzztest(test_${ALGO})

        gtest_discover_tests(test_${ALGO})
    endif()
endforeach()
