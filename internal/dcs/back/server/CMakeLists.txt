cmake_minimum_required(VERSION 3.16)
project(server)

set(HEADERS include/storage/storage.h include/playground/executor.h)

set(SOURCES
        src/storage/storage.cpp
        src/playground/executor.cpp
        )

add_executable(dcsserver
        src/main.cpp
        src/options/options.cpp
        ${SOURCES}
        ${HEADERS}
        )
target_link_libraries(dcsserver pthread glog::glog httplib lang common)
target_include_directories(dcsserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(dcs_test_generator ${HEADERS} ${SOURCES} src/test_generator/main.cpp)
target_include_directories(dcs_test_generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dcs_test_generator pthread glog::glog common lang)

add_executable(dcsserver_tests
        tests/main.cpp
        tests/storage/storage_tests.cpp
        tests/playground/executor_tests.cpp
        ${SOURCES}
        ${HEADERS}
        )
target_include_directories(dcsserver_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dcsserver_tests pthread glog::glog gtest gtest_main common lang)
