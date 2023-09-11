CXX=g++
CXXFLAGS=-Wall -std=c++11 -g -pthread -O3 -D NDEBUG
LIB=libaiger.a

HEADERS=aiger.h
SOURCES=aiger.c
OBJECTS=$(SOURCES:.c=.o)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $^

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $(OBJECTS)

clean: 
	rm -rf $(LIB) $(OBJECTS)

all: clean $(LIB)

.PHONY: clean
