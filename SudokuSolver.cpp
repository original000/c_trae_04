#include "SudokuSolver.h"
#include <iostream>
#include <iomanip>

using namespace std;

SudokuSolver::SudokuSolver() {
    // Initialize board to all zeros
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            board[i][j] = 0;
        }
    }
}

SudokuSolver::~SudokuSolver() {
    // No dynamic memory allocation, so nothing to do here
}

void SudokuSolver::setBoard(int inputBoard[9][9]) {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            board[i][j] = inputBoard[i][j];
        }
    }
}

void SudokuSolver::printBoard() {
    cout << "┌─────────┬─────────┬─────────┐" << endl;
    for (int i = 0; i < 9; ++i) {
        cout << "│ ";
        for (int j = 0; j < 9; ++j) {
            if (board[i][j] == 0) {
                cout << "  ";
            } else {
                cout << board[i][j] << " ";
            }
            if ((j + 1) % 3 == 0 && j != 8) {
                cout << "│ ";
            }
        }
        cout << "│" << endl;
        if ((i + 1) % 3 == 0 && i != 8) {
            cout << "├─────────┼─────────┼─────────┤" << endl;
        }
    }
    cout << "└─────────┴─────────┴─────────┘" << endl;
}

bool SudokuSolver::isSafe(int row, int col, int num) {
    // Check if num is already in current row
    for (int x = 0; x < 9; ++x) {
        if (board[row][x] == num) {
            return false;
        }
    }
    
    // Check if num is already in current column
    for (int x = 0; x < 9; ++x) {
        if (board[x][col] == num) {
            return false;
        }
    }
    
    // Check if num is already in current 3x3 box
    int boxRowStart = row - row % 3;
    int boxColStart = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[boxRowStart + i][boxColStart + j] == num) {
                return false;
            }
        }
    }
    
    return true;
}

bool SudokuSolver::solveSudoku() {
    int row, col;
    bool found = false;
    
    // Find the first empty cell
    for (row = 0; row < 9; ++row) {
        for (col = 0; col < 9; ++col) {
            if (board[row][col] == 0) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    
    // If no empty cells, puzzle is solved
    if (!found) {
        return true;
    }
    
    // Try numbers 1 to 9
    for (int num = 1; num <= 9; ++num) {
        if (isSafe(row, col, num)) {
            // Tentatively place num
            board[row][col] = num;
            
            // Recursively solve the rest of the puzzle
            if (solveSudoku()) {
                return true;
            }
            
            // If placing num doesn't lead to a solution, backtrack
            board[row][col] = 0;
        }
    }
    
    // If no number works, return false (trigger backtracking)
    return false;
}

void SudokuSolver::countSolutionsHelper(int &count) {
    int row, col;
    bool found = false;
    
    // Find the first empty cell
    for (row = 0; row < 9; ++row) {
        for (col = 0; col < 9; ++col) {
            if (board[row][col] == 0) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    
    // If no empty cells, we found a solution
    if (!found) {
        count++;
        return;
    }
    
    // Try numbers 1 to 9
    for (int num = 1; num <= 9; ++num) {
        if (isSafe(row, col, num)) {
            // Tentatively place num
            board[row][col] = num;
            
            // Recursively check all possibilities
            countSolutionsHelper(count);
            
            // Backtrack
            board[row][col] = 0;
            
            // Early exit if we found more than 1 solution (no need to count all)
            if (count > 1) {
                return;
            }
        }
    }
}

int SudokuSolver::countSolutions() {
    int count = 0;
    countSolutionsHelper(count);
    return count;
}