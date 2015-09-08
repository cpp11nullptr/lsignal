# 
# Makefile for lsignal
#

CXX=g++
CXXFLAGS=-std=c++11 -c -O3 -Wall
LDFLAGS=
EXECUTABLE=lsignal
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)

