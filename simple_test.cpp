#include "SudokuSolver.h"
#include <iostream>

using namespace std;

// 测试用的数独难题
int testBoard1[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
}; // Unique solution

int testBoard2[9][9] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}
}; // Multiple solutions

int testBoard3[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 8}
}; // No solution (two 8s in last row)

int main() {
    SudokuSolver solver;
    bool allTestsPassed = true;
    
    // Test 1: Unique solution
    solver.setBoard(testBoard1);
    int count1 = solver.countSolutions();
    if (count1 == 1) {
        cout << "Test 1: PASSED" << endl;
    } else {
        cout << "Test 1: FAILED (expected 1 solution, found " << count1 << ")" << endl;
        allTestsPassed = false;
    }
    
    // Test 2: Multiple solutions
    solver.setBoard(testBoard2);
    int count2 = solver.countSolutions();
    if (count2 > 1) {
        cout << "Test 2: PASSED" << endl;
    } else {
        cout << "Test 2: FAILED (expected multiple solutions, found " << count2 << ")" << endl;
        allTestsPassed = false;
    }
    
    // Test 3: No solution
    solver.setBoard(testBoard3);
    int count3 = solver.countSolutions();
    if (count3 == 0) {
        cout << "Test 3: PASSED" << endl;
    } else {
        cout << "Test 3: FAILED (expected no solution, found " << count3 << ")" << endl;
        allTestsPassed = false;
    }
    
    // Test 4: Solve function
    solver.setBoard(testBoard1);
    bool solved = solver.solveSudoku();
    if (solved) {
        cout << "Test 4: PASSED (solve function works)" << endl;
    } else {
        cout << "Test 4: FAILED (solve function returned false for solvable puzzle)" << endl;
        allTestsPassed = false;
    }
    
    cout << endl;
    if (allTestsPassed) {
        cout << "All tests passed!" << endl;
    } else {
        cout << "Some tests failed!" << endl;
    }
    
    return 0;
}