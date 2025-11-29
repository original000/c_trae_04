#include "Encryptor.h"
#include <fstream>
#include <iostream>

std::string Encryptor::encrypt(const std::string& text, int shift, const std::string& key) {
    std::string result;
    for (size_t i = 0; i < text.size(); ++i) {
        char shifted = caesarShift(text[i], shift);
        char keyChar = getKeyChar(key, i);
        result += (shifted ^ keyChar);
    }
    return result;
}

std::string Encryptor::decrypt(const std::string& encrypted, int shift, const std::string& key) {
    std::string result;
    for (size_t i = 0; i < encrypted.size(); ++i) {
        char keyChar = getKeyChar(key, i);
        char decryptedChar = (encrypted[i] ^ keyChar);
        result += caesarShift(decryptedChar, -shift);
    }
    return result;
}

bool Encryptor::encryptFile(const std::string& inputFile, const std::string& outputFile, int shift, const std::string& key) {
    std::ifstream inFile(inputFile.c_str(), std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Input file \"" << inputFile << "\" not found." << std::endl;
        return false;
    }
    
    // 读取整个文件内容
    std::string content((std::istreambuf_iterator<char>(inFile)),
                         std::istreambuf_iterator<char>());
    inFile.close();
    
    // 加密内容
    std::string encrypted = encrypt(content, shift, key);
    
    // 写入输出文件
    std::ofstream outFile(outputFile.c_str(), std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not create output file \"" << outputFile << "\" ." << std::endl;
        return false;
    }
    
    outFile.write(encrypted.c_str(), encrypted.size());
    outFile.close();
    
    return true;
}

bool Encryptor::decryptFile(const std::string& inputFile, const std::string& outputFile, int shift, const std::string& key) {
    std::ifstream inFile(inputFile.c_str(), std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Input file \"" << inputFile << "\" not found." << std::endl;
        return false;
    }
    
    // 读取整个文件内容
    std::string content((std::istreambuf_iterator<char>(inFile)),
                         std::istreambuf_iterator<char>());
    inFile.close();
    
    // 解密内容
    std::string decrypted = decrypt(content, shift, key);
    
    // 写入输出文件
    std::ofstream outFile(outputFile.c_str(), std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not create output file \"" << outputFile << "\" ." << std::endl;
        return false;
    }
    
    outFile.write(decrypted.c_str(), decrypted.size());
    outFile.close();
    
    return true;
}

char Encryptor::caesarShift(char c, int shift) {
    if (isalpha(c)) {
        char base = islower(c) ? 'a' : 'A';
        // 处理负移位和大移位值
        shift = shift % 26;
        if (shift < 0) {
            shift += 26;
        }
        
        char shifted = (c - base + shift) % 26 + base;
        return shifted;
    }
    // 非字母字符保持不变
    return c;
}

char Encryptor::getKeyChar(const std::string& key, size_t index) {
    if (key.empty()) {
        return "secret"[index % 6];
    }
    return key[index % key.size()];
}