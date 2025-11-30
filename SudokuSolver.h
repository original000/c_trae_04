#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

class SudokuSolver {
public:
    SudokuSolver();
    ~SudokuSolver();
    
    void setBoard(int board[9][9]);
    void printBoard();
    bool solveSudoku();
    int countSolutions();
    
private:
    int board[9][9];
    bool isSafe(int row, int col, int num);
    void countSolutionsHelper(int &count);
};

#endif // SUDOKUSOLVER_H