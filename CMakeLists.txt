cmake_minimum_required(VERSION 3.16)
project(instruction_encoder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

enable_testing()

add_library(encoder_lib
    src/parser.cpp
    src/validator.cpp
    src/allocator.cpp
    src/generator.cpp
    src/utils.cpp
)

target_include_directories(encoder_lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(encoder_lib
    PUBLIC
        nlohmann_json::nlohmann_json
)

add_executable(instruction_encoder
    src/main.cpp
)

target_link_libraries(instruction_encoder
    PRIVATE
        encoder_lib
)

set_target_properties(instruction_encoder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(TEST_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test)

add_executable(test_allocator
    tests/test_allocator.cpp
)

target_link_libraries(test_allocator
    PRIVATE
        encoder_lib
)

set_target_properties(test_allocator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)

add_executable(test_integration
    tests/test_integration.cpp
)

target_link_libraries(test_integration
    PRIVATE
        encoder_lib
)

set_target_properties(test_integration PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)

add_executable(test_impossible
    tests/test_impossible.cpp
)

target_link_libraries(test_impossible
    PRIVATE
        encoder_lib
)

set_target_properties(test_impossible PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)

# CTest
add_test(NAME allocator_test COMMAND ${TEST_OUTPUT_DIR}/test_allocator)
set_tests_properties(allocator_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_test(NAME integration_test COMMAND ${TEST_OUTPUT_DIR}/test_integration)
set_tests_properties(integration_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_test(NAME impossible_test COMMAND ${TEST_OUTPUT_DIR}/test_impossible)
set_tests_properties(impossible_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)