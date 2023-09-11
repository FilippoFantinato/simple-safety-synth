CXX=g++
CXXFLAGS=-Wall -std=c++11 -g -pthread -O3 -D NDEBUG
EXEC=simple-safety-synth.out

AIGER_PATH=aiger
AIGER_LIBS=$(AIGER_PATH)/libaiger.a

CUDD_PATH=cudd
CUDD_HDRS=$(CUDD_PATH)/include
CUDD_LIBS=$(CUDD_PATH)/obj/libobj.a \
	  $(CUDD_PATH)/cudd/libcudd.a \
	  $(CUDD_PATH)/mtr/libmtr.a \
	  $(CUDD_PATH)/st/libst.a \
	  $(CUDD_PATH)/util/libutil.a \
	  $(CUDD_PATH)/epd/libepd.a

HEADERS=safety-arena/SafetyArena.h safety-solver/SafetySolver.h safety-arena/aiger-utils.h # safety-arena/BDD2Aiger.h
SOURCES=main.cpp safety-arena/SafetyArena.cpp safety-arena/aiger-utils.cpp
OBJECTS=$(SOURCES:.cpp=.o)

$(EXEC): $(HEADERS) $(SOURCES) $(CUDD_HDRS) $(CUDD_LIBS) $(AIGER_LIBS)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(CUDD_LIBS) $(AIGER_LIBS) -o $(EXEC) -I $(CUDD_HDRS)

clean:
	rm -f $(EXEC) $(OBJECTS)

all: clean $(EXEC)
