cmake_minimum_required(VERSION 3.10)

project(simple-synth)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O3 -g -pthread -D NDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")
set(CMAKE_SHARED_LINKER_FLAGS "-lm -pthread -ldl -lcudd")

find_package(argparse REQUIRED)
find_library(cudd REQUIRED NAMES libcudd.a)
find_program(BASH_EXECUTABLE NAMES bash REQUIRED)

set(AIGER_LIBS ${CMAKE_SOURCE_DIR}/src/aiger/libaiger.a)

add_executable(simple-synth
    main.cpp
    src/safety-arena/SafetyArena.h
    src/safety-arena/SafetyArena.cpp
    src/solvers/BDD2Aiger.h
    src/solvers/BDD2Aiger.cpp
    src/solvers/GameSolver.h
    src/solvers/GameSolver.cpp
    src/solvers/SimpleSafetySolver.h
    src/solvers/SimpleSafetySolver.cpp
    src/solvers/GFPSafetySolver.h
    src/solvers/GFPSafetySolver.cpp
    src/solvers/SimpleCoSafetySolver.h
    src/solvers/SimpleCoSafetySolver.cpp
    src/solvers/SimpleCoSafetySolver2.h
    src/solvers/SimpleCoSafetySolver2.cpp
    src/utils/aiger.h
    src/utils/aiger.cpp
    src/utils/aiger2smv.cpp
)

target_link_libraries(simple-synth PRIVATE
    cudd
    ${AIGER_LIBS}
    argparse::argparse
)

target_include_directories(simple-synth PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/aiger
)

enable_testing()

add_test(
    NAME SafetySystemTest
    COMMAND bash ../tests/system/safety-system-test.sh
)

add_test(
    NAME CoSafetySystemTest
    COMMAND bash ../tests/system/co-safety-system-test.sh
)
