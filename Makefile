# Makefile for Sudoku Solver

CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

TARGET = sudoku_solver

SOURCES = SudokuSolver.cpp main.cpp
HEADERS = SudokuSolver.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)