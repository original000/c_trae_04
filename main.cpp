#include "SudokuSolver.h"
#include <iostream>
#include <string>
#include <sstream>
#include <limits>

using namespace std;

void inputBoard(int board[9][9]) {
    cout << "请输入9行，每行9个数字（空格分隔，0表示空）：" << endl;
    cout << "提示：支持直接粘贴9行数据" << endl;
    
    for (int i = 0; i < 9; ++i) {
        string line;
        bool valid = false;
        
        while (!valid) {
            cout << "第" << (i + 1) << "行：";
            getline(cin, line);
            
            // 处理可能的空行
            if (line.empty()) {
                continue;
            }
            
            istringstream iss(line);
            int num;
            int count = 0;
            
            while (iss >> num) {
                if (num >= 0 && num <= 9) {
                    board[i][count] = num;
                    count++;
                } else {
                    break; // 无效数字
                }
            }
            
            // 检查是否读取了9个有效数字
            if (count == 9) {
                valid = true;
            } else {
                cout << "输入无效！请输入9个0-9之间的数字（空格分隔）。" << endl;
            }
        }
    }
}

int main() {
    SudokuSolver solver;
    int board[9][9];
    char continueChoice;
    
    do {
        // 清空输入缓冲区
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        // 输入数独
        inputBoard(board);
        
        // 设置数独并统计解的个数
        solver.setBoard(board);
        int solutionCount = solver.countSolutions();
        
        // 根据解的个数输出结果
        if (solutionCount == 0) {
            cout << "无解" << endl;
        } else if (solutionCount == 1) {
            cout << "唯一解：" << endl;
            solver.setBoard(board); // 重新设置初始状态
            solver.solveSudoku();
            solver.printBoard();
        } else {
            cout << "有多解，共" << solutionCount << "个" << endl;
        }
        
        // 询问是否继续
        cout << "是否继续输入下一题？(y/n)：";
        cin >> continueChoice;
        
    } while (continueChoice == 'y' || continueChoice == 'Y');
    
    cout << "感谢使用数独求解器！" << endl;
    
    return 0;
}