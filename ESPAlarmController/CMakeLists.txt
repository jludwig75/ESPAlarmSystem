cmake_minimum_required(VERSION 3.0.0)
project(test_sorts VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Catch2)

include(CTest)
enable_testing()

add_library(test_main STATIC
            test_main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(test)