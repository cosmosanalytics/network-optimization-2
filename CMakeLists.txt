cmake_minimum_required(VERSION 3.16)
project(network_optimization_2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(USE_CBC "Build the CbcMipSolver backend (requires COIN-OR CBC dev libraries)" OFF)

include_directories(include)

set(CORE_SOURCES
    src/PackingProblem.cpp
    src/GreedyFirstFitDecreasingSolver.cpp
    src/BranchAndBoundSolver.cpp
)

add_library(network_opt2_core ${CORE_SOURCES})
target_include_directories(network_opt2_core PUBLIC include)

if(USE_CBC)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CBC REQUIRED cbc)
  target_compile_definitions(network_opt2_core PUBLIC NETOPT2_USE_CBC)
  target_include_directories(network_opt2_core PUBLIC ${CBC_INCLUDE_DIRS})
  target_link_libraries(network_opt2_core PUBLIC ${CBC_LIBRARIES})
endif()

add_executable(network_opt2_demo src/main.cpp)
target_link_libraries(network_opt2_demo PRIVATE network_opt2_core)

add_executable(network_opt2_tests
    tests/main.cpp
    tests/PackingTests.cpp
)
target_include_directories(network_opt2_tests PRIVATE tests)
target_link_libraries(network_opt2_tests PRIVATE network_opt2_core)

enable_testing()
add_test(NAME unit_tests COMMAND network_opt2_tests)
