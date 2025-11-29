#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <string>

class Encryptor {
public:
    // 加密字符串：Caesar 移位 + XOR 密钥
    static std::string encrypt(const std::string& text, int shift, const std::string& key);
    
    // 解密字符串：逆 Caesar 移位 + XOR 密钥
    static std::string decrypt(const std::string& encrypted, int shift, const std::string& key);
    
    // 加密文件
    static bool encryptFile(const std::string& inputFile, const std::string& outputFile, int shift, const std::string& key);
    
    // 解密文件
    static bool decryptFile(const std::string& inputFile, const std::string& outputFile, int shift, const std::string& key);
    
private:
    // 辅助函数：对单个字符进行 Caesar 移位
    static char caesarShift(char c, int shift);
    
    // 辅助函数：获取密钥中指定位置的字符（循环使用密钥）
    static char getKeyChar(const std::string& key, size_t index);
};

#endif // ENCRYPTOR_H