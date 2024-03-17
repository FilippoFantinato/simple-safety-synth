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

set(AIGER_LIBS ${CMAKE_SOURCE_DIR}/src/aiger/libaiger.a)

add_executable(simple-safety-synth
    main.cpp
    src/safety-arena/SafetyArena.h
    src/safety-arena/SafetyArena.cpp
    src/safety-solver/BDD2Aiger.h
    src/safety-solver/BDD2Aiger.cpp
    src/safety-solver/GameSolver.h
    src/safety-solver/GameSolver.cpp
    src/safety-solver/SimpleSafetySolver.h
    src/safety-solver/SimpleSafetySolver.cpp
    src/safety-solver/GFPSafetySolver.h
    src/safety-solver/GFPSafetySolver.cpp
    src/safety-solver/SimpleCoSafetySolver.h
    src/safety-solver/SimpleCoSafetySolver.cpp
    src/utils/aiger.h
    src/utils/aiger.cpp
    src/utils/aiger2smv.cpp
)

target_link_libraries(simple-safety-synth PRIVATE
    cudd
    ${AIGER_LIBS}
    argparse::argparse
)

target_include_directories(simple-safety-synth PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/aiger
)

enable_testing()

add_test(
    NAME SystemTests
    COMMAND bash ../tests/system/run-system-test.sh
)
