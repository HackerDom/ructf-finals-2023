cmake_minimum_required(VERSION 3.16)
project(back)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Werror")

add_executable(dcs
        src/main.cpp
        src/compiler/lexer.cpp
        src/compiler/parser.cpp
        src/compiler/ast.cpp
        src/utils/utils.h
        src/utils/utils.cpp
        src/compiler/compiler.cpp
        src/compiler/translator.cpp)
target_link_libraries(dcs dl)
target_include_directories(dcs PUBLIC src)

add_subdirectory(googletest)
add_executable(dcs_tests
        src/tests/lexer_tests.cpp
        src/tests/parser_tests.cpp
        src/compiler/lexer.cpp
        src/compiler/parser.cpp
        src/compiler/ast.cpp
        src/compiler/compiler.cpp
        src/utils/utils.cpp
        src/tests/compiler_tests.cpp
        src/tests/functional_tests.cpp
        src/compiler/translator.cpp)
target_include_directories(dcs_tests PUBLIC src)
target_link_libraries(dcs_tests GTest::gtest_main dl)
include(GoogleTest)
gtest_discover_tests(dcs_tests)
