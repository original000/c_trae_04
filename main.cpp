#include "Encryptor.h"
#include <iostream>
#include <string>
#include <fstream>
#include <limits>

int main() {
    char mode;
    std::string inputFile, outputFile, key;
    int shift;
    
    // 输入模式
    std::cout << "Enter mode (e for encrypt, d for decrypt): ";
    std::cin >> mode;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略输入缓冲区中的所有剩余字符，直到换行符
    
    // 验证模式
    if (mode != 'e' && mode != 'd') {
        std::cerr << "Error: Invalid mode. Please enter 'e' for encrypt or 'd' for decrypt." << std::endl;
        return 1;
    }
    
    // 输入文件路径
    std::cout << "Enter input file path: ";
    std::getline(std::cin, inputFile);
    
    // 输入输出文件路径
    std::cout << "Enter output file path: ";
    std::getline(std::cin, outputFile);
    
    // 输入移位数
    std::cout << "Enter shift value (1-25): ";
    std::cin >> shift;
    std::cin.ignore(); // 忽略输入缓冲区中的换行符
    
    // 验证移位数
    if (shift < 1 || shift > 25) {
        std::cerr << "Error: Invalid shift value. Please enter a number between 1 and 25." << std::endl;
        return 1;
    }
    
    // 输入密钥
    std::cout << "Enter key (leave empty to use default 'secret'): ";
    std::getline(std::cin, key);
    
    // 如果密钥为空，使用默认密钥 "secret"
    if (key.empty()) {
        key = "secret";
    }
    
    bool success = false;
    size_t charsProcessed = 0;
    
    // 执行加密或解密
    if (mode == 'e') {
        success = Encryptor::encryptFile(inputFile, outputFile, shift, key);
    } else {
        success = Encryptor::decryptFile(inputFile, outputFile, shift, key);
    }
    
    // 检查操作是否成功
    if (success) {
        // 计算处理的字符数
        std::ifstream inFile(inputFile.c_str(), std::ios::binary | std::ios::ate);
        if (inFile) {
            charsProcessed = inFile.tellg();
            inFile.close();
        }
        
        // 显示成功消息
        if (mode == 'e') {
            std::cout << "Encryption complete: " << charsProcessed << " chars processed." << std::endl;
        } else {
            std::cout << "Decryption complete: " << charsProcessed << " chars processed." << std::endl;
        }
    } else {
        // 显示失败消息
        if (mode == 'e') {
            std::cerr << "Encryption failed." << std::endl;
        } else {
            std::cerr << "Decryption failed." << std::endl;
        }
        return 1;
    }
    
    return 0;
}