# 
# Makefile for lsignal
#

CXX?=g++
CXXFLAGS?=-std=c++11 -O3 -Wall
LDFLAGS?=
EXECUTABLE=lsignal
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: all
all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.cpp lsignal.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)

