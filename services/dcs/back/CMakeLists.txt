cmake_minimum_required(VERSION 3.16)
project(back)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Werror")

add_executable(dcs
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/utils.h
    src/utils.cpp
    src/compiler.cpp
)
target_link_libraries(dcs dl)

add_subdirectory(googletest)
enable_testing()
add_executable(dcs_tests
    src/lexer_tests.cpp
    src/parser_tests.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/compiler.cpp
    src/utils.cpp
    src/compiler_tests.cpp
    src/functional_tests.cpp
)
target_link_libraries(dcs_tests GTest::gtest_main dl)
include(GoogleTest)
gtest_discover_tests(dcs_tests)
