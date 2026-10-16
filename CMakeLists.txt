cmake_minimum_required(VERSION 3.21)
project(geomera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wpedantic>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wconversion>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wshadow>
)

find_package(Eigen3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_library(geomera_core STATIC
    src/placeholder.cpp
    src/measurements.cpp
)
target_include_directories(geomera_core PUBLIC include)
target_link_libraries(geomera_core PUBLIC
    Eigen3::Eigen
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    tl::expected
)

add_executable(meshtool
    cli/main.cpp
)
target_link_libraries(meshtool PRIVATE
    geomera_core
    CLI11::CLI11
)

enable_testing()

add_library(geomera_test_support STATIC
    tests/support/test_helpers.cpp
)
target_include_directories(geomera_test_support PUBLIC tests/support)
target_link_libraries(geomera_test_support PUBLIC
    geomera_core
    GTest::gtest
)

add_executable(geomera_tests
    tests/unit/placeholder_test.cpp
    tests/unit/mesh_test.cpp
    tests/unit/measurements_test.cpp
    tests/unit/error_test.cpp
)
target_link_libraries(geomera_tests PRIVATE
    geomera_test_support
    GTest::gtest_main
)
add_test(NAME geomera_tests COMMAND geomera_tests)

add_executable(geomera_bench
    bench/placeholder_bench.cpp
)
target_link_libraries(geomera_bench PRIVATE
    geomera_core
    benchmark::benchmark
    benchmark::benchmark_main
)
