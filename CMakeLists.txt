cmake_minimum_required(VERSION 3.10)

project(simple-safety-synth)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O3 -g -pthread -D NDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")
set(CMAKE_SHARED_LINKER_FLAGS "-lm -pthread -ldl -lcudd")

find_package(argparse REQUIRED)
find_library(cudd REQUIRED NAMES libcudd.a)
find_program(BASH_EXECUTABLE NAMES bash REQUIRED)

set(AIGER_LIBS ${CMAKE_SOURCE_DIR}/aiger/libaiger.a)

add_executable(simple-safety-synth
    main.cpp
    safety-arena/SafetyArena.h
    safety-arena/SafetyArena.cpp
    safety-solver/BDD2Aiger.h
    safety-solver/BDD2Aiger.cpp
    safety-solver/GameSolver.h
    safety-solver/GameSolver.cpp
    safety-solver/SimpleSafetySolver.h
    safety-solver/SimpleSafetySolver.cpp
    safety-solver/GFPSafetySolver.h
    safety-solver/GFPSafetySolver.cpp
    safety-solver/SimpleCoSafetySolver.h
    safety-solver/SimpleCoSafetySolver.cpp
    utils/aiger.h
    utils/aiger.cpp
    utils/aiger2smv.cpp
)

target_link_libraries(simple-safety-synth PRIVATE
    cudd
    ${AIGER_LIBS}
    argparse::argparse
)

target_include_directories(simple-safety-synth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/aiger)

enable_testing()

add_test(
    NAME SystemTests
    COMMAND bash ../tests/system/run-system-test.sh
)
