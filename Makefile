# makefile for the wave1d application

CXX=g++
CXXFLAGS=-O2 -g -std=c++17 -Wall -Wfatal-errors -Wconversion
LDFLAGS=-O2 -g
all: wave1d

wave1d: wave1d.o
	$(CXX) $(LDFLAGS) -o wave1d wave1d.o

wave1d.o: wave1d.cpp
	$(CXX) -c $(CXXFLAGS) -o wave1d.o wave1d.cpp

run: wave1d
	./wave1d waveparams.txt

clean:
	$(RM) wave1d.o

.PHONY: all clean run

