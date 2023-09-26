cmake_minimum_required(VERSION 3.10)

project(simple-safety-synth)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -g -pthread -O3 -D NDEBUG")
set(CMAKE_MODULE_LINKER_FLAGS "-lm -pthread -ldl")

find_package(argparse REQUIRED)

find_program(BASH_EXECUTABLE NAMES bash REQUIRED)

set(CUDD_PATH ${CMAKE_SOURCE_DIR}/cudd)
set(CUDD_LIBS
    ${CUDD_PATH}/obj/libobj.a 
    ${CUDD_PATH}/cudd/libcudd.a 
    ${CUDD_PATH}/mtr/libmtr.a 
    ${CUDD_PATH}/st/libst.a 
    ${CUDD_PATH}/util/libutil.a
    ${CUDD_PATH}/epd/libepd.a
)
set(AIGER_PATH ${CMAKE_SOURCE_DIR}/aiger)
set(AIGER_LIBS ${AIGER_PATH}/libaiger.a)

include_directories(${CUDD_PATH}/include)

add_executable(simple-safety-synth
    main.cpp
    safety-arena/SafetyArena.h
    safety-arena/SafetyArena.cpp
    safety-solver/BDD2Aiger.h
    safety-solver/BDD2Aiger.cpp
    safety-solver/SafetySolver.h
    safety-solver/SafetySolver.cpp
    safety-solver/SimpleSafetySolver.h
    safety-solver/SimpleSafetySolver.cpp
    safety-solver/BetterSafetySolver.h
    safety-solver/BetterSafetySolver.cpp
    utils/aiger.h
    utils/aiger.cpp
    utils/aiger2smv.cpp
)

target_link_libraries(simple-safety-synth
    ${CUDD_LIBS}
    ${AIGER_LIBS}
    argparse::argparse
)

enable_testing()

add_test(
    NAME SystemTests
    COMMAND sh ../tests/system/run-system-test.sh
)
