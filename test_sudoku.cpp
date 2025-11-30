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
    
    cout << "Test 1: Sudoku puzzle with unique solution" << endl;
    solver.setBoard(testBoard1);
    int count1 = solver.countSolutions();
    if (count1 == 1) {
        cout << "✓ Correct: has unique solution" << endl;
    } else {
        cout << "✗ Error: expected unique solution, but found " << count1 << " solutions" << endl;
    }
    
    cout << endl << "Test 2: Empty Sudoku (multiple solutions)" << endl;
    solver.setBoard(testBoard2);
    int count2 = solver.countSolutions();
    if (count2 > 1) {
        cout << "✓ Correct: has multiple solutions (" << count2 << " solutions found)" << endl;
    } else {
        cout << "✗ Error: expected multiple solutions, but found " << count2 << " solutions" << endl;
    }
    
    cout << endl << "Test 3: Sudoku puzzle with no solution" << endl;
    solver.setBoard(testBoard3);
    int count3 = solver.countSolutions();
    if (count3 == 0) {
        cout << "✓ Correct: has no solution" << endl;
    } else {
        cout << "✗ Error: expected no solution, but found " << count3 << " solutions" << endl;
    }
    
    cout << endl << "All tests completed!" << endl;
    
    return 0;
}