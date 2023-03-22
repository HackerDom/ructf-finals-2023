cmake_minimum_required(VERSION 3.16)
project(back)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

add_executable(dcs
    src/main.cpp
    src/parser/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast.cpp
)

add_subdirectory(googletest)
enable_testing()
add_executable(dcs_tests
    src/lexer_tests.cpp
    src/parser_tests.cpp
    src/parser/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast.cpp
)
target_link_libraries(dcs_tests GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(dcs_tests)
