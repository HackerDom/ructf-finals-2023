cmake_minimum_required(VERSION 3.16)
project(lang)

set(HEADERS
        include/ast/ast.h
        include/compiler/compiler.h
        include/lexer/lexer.h
        include/parser/parser.h
        include/translator/translator.h
        )

set(SOURCES
        src/ast/ast.cpp
        src/compiler/compiler.cpp
        src/lexer/lexer.cpp
        src/parser/parser.cpp
        src/translator/translator.cpp
        )

add_library(lang ${HEADERS} ${SOURCES})
target_include_directories(lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lang common)

add_executable(dcslang ${HEADERS} ${SOURCES} src/main.cpp)
target_include_directories(dcslang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dcslang common)

add_executable(dcslang_tests
        ${HEADERS}
        ${SOURCES}
        tests/compiler/compiler_tests.cpp
        tests/lexer/lexer_tests.cpp
        tests/parser/parser_tests.cpp
        tests/functional_tests.cpp
        tests/performance_tests.cpp
        tests/main.cpp)
target_include_directories(dcslang_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dcslang_tests glog::glog gtest gtest_main common)
