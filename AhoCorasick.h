#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>

struct Node {
    std::unordered_map<char, int> children;
    int fail; // 失败转移指针
    std::vector<int> outputs; // 输出链表，存储匹配到的规则ID
    Node() : fail(0) {}
};

class AhoCorasick {
public:
    AhoCorasick(bool case_insensitive = true);
    ~AhoCorasick();
    
    // 添加规则，返回规则ID
    int addRule(const std::string& pattern);
    
    // 构建失败转移指针和输出链表
    void build();
    
    // 匹配文本，返回所有命中的规则ID和位置
    std::vector<std::pair<int, int>> match(const std::string& text);
    
    // 获取规则数量
    int getRuleCount() const;
    
private:
    std::vector<Node*> nodes;
    std::vector<std::string> patterns;
    bool case_insensitive;
    
    // 转换字符为小写（如果忽略大小写）
    char toLower(char c) const;
};

#endif // AHO_CORASICK_H