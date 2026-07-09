cmake_minimum_required(VERSION 3.24)
project(my_project)

cmake_policy(SET CMP0135 NEW)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  main
  main.cpp
  lib.cpp
)
if(MSVC)
    target_compile_options(main PRIVATE /W4)
endif()

add_executable(
  lib_test
  lib_test.cpp
  lib.cpp
)
target_link_libraries(
  lib_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lib_test)
