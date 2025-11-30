#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>

#include "src/Database.h"

void test_string_operations() {
    std::cout << "Testing string operations...\n";

    Database db;

    // 测试SET和GET
    assert(db.set("key1", "value1") == true);
    std::string* value = db.get("key1");
    assert(value != nullptr);
    assert(*value == "value1");
    delete value;

    // 测试SET覆盖
    assert(db.set("key1", "value2") == true);
    value = db.get("key1");
    assert(value != nullptr);
    assert(*value == "value2");
    delete value;

    // 测试SET with EXPIRE
    assert(db.set("key2", "value2", 1) == true);
    value = db.get("key2");
    assert(value != nullptr);
    assert(*value == "value2");
    delete value;

    // 等待过期
    std::this_thread::sleep_for(std::chrono::seconds(2));
    value = db.get("key2");
    assert(value == nullptr);
    if (value) delete value;

    // 测试DEL
    assert(db.set("key3", "value3") == true);
    assert(db.del("key3") == true);
    value = db.get("key3");
    assert(value == nullptr);
    if (value) delete value;

    // 测试DEL不存在的键
    assert(db.del("nonexistent") == false);

    std::cout << "String operations test passed!\n\n";
}

void test_hash_operations() {
    std::cout << "Testing hash operations...\n";

    Database db;

    // 测试HSET和HGET
    assert(db.hset("hash1", "field1", "value1") == true);
    std::string* value = db.hget("hash1", "field1");
    assert(value != nullptr);
    assert(*value == "value1");
    delete value;

    // 测试HSET覆盖
    assert(db.hset("hash1", "field1", "value2") == true);
    value = db.hget("hash1", "field1");
    assert(value != nullptr);
    assert(*value == "value2");
    delete value;

    // 测试HSET多个字段
    assert(db.hset("hash1", "field2", "value3") == true);
    value = db.hget("hash1", "field2");
    assert(value != nullptr);
    assert(*value == "value3");
    delete value;

    // 测试HGET不存在的字段
    value = db.hget("hash1", "nonexistent");
    assert(value == nullptr);
    if (value) delete value;

    // 测试HGET不存在的哈希
    value = db.hget("nonexistent", "field1");
    assert(value == nullptr);
    if (value) delete value;

    // 测试DEL哈希
    assert(db.del("hash1") == true);
    value = db.hget("hash1", "field1");
    assert(value == nullptr);
    if (value) delete value;

    std::cout << "Hash operations test passed!\n\n";
}

void test_list_operations() {
    std::cout << "Testing list operations...\n";

    Database db;

    // 测试LPUSH和LRANGE
    assert(db.lpush("list1", "value1") == true);
    std::vector<std::string>* list = db.lrange("list1", 0, -1);
    assert(list != nullptr);
    assert(list->size() == 1);
    assert((*list)[0] == "value1");
    delete list;

    // 测试LPUSH多个元素
    assert(db.lpush("list1", "value2") == true);
    assert(db.lpush("list1", "value3") == true);
    list = db.lrange("list1", 0, -1);
    assert(list != nullptr);
    assert(list->size() == 3);
    assert((*list)[0] == "value3");
    assert((*list)[1] == "value2");
    assert((*list)[2] == "value1");
    delete list;

    // 测试LRANGE范围
    list = db.lrange("list1", 0, 1);
    assert(list != nullptr);
    assert(list->size() == 2);
    assert((*list)[0] == "value3");
    assert((*list)[1] == "value2");
    delete list;

    list = db.lrange("list1", 1, 1);
    assert(list != nullptr);
    assert(list->size() == 1);
    assert((*list)[0] == "value2");
    delete list;

    // 测试LRANGE超出范围
    list = db.lrange("list1", 0, 10);
    assert(list != nullptr);
    assert(list->size() == 3);
    delete list;

    list = db.lrange("list1", 10, 20);
    assert(list != nullptr);
    assert(list->size() == 0);
    delete list;

    // 测试LRANGE不存在的列表
    list = db.lrange("nonexistent", 0, -1);
    assert(list == nullptr);
    if (list) delete list;

    // 测试DEL列表
    assert(db.del("list1") == true);
    list = db.lrange("list1", 0, -1);
    assert(list == nullptr);
    if (list) delete list;

    std::cout << "List operations test passed!\n\n";
}

void test_type_safety() {
    std::cout << "Testing type safety...\n";

    Database db;

    // 测试不同类型之间的冲突
    assert(db.set("key1", "string_value") == true);

    // 尝试对字符串键执行哈希操作
    assert(db.hset("key1", "field1", "value1") == false);
    std::string* value = db.hget("key1", "field1");
    assert(value == nullptr);
    if (value) delete value;

    // 尝试对字符串键执行列表操作
    assert(db.lpush("key1", "list_value") == false);
    std::vector<std::string>* list = db.lrange("key1", 0, -1);
    assert(list == nullptr);
    if (list) delete list;

    // 清理
    assert(db.del("key1") == true);

    // 测试哈希键的类型冲突
    assert(db.hset("hash1", "field1", "value1") == true);

    // 尝试对哈希键执行字符串操作
    assert(db.set("hash1", "string_value") == true); // 应该允许覆盖
    value = db.get("hash1");
    assert(value != nullptr);
    assert(*value == "string_value");
    delete value;

    // 清理
    assert(db.del("hash1") == true);

    std::cout << "Type safety test passed!\n\n";
}

int main() {
    std::cout << "=== MiniRedis Test Suite ===\n\n";

    try {
        test_string_operations();
        test_hash_operations();
        test_list_operations();
        test_type_safety();

        std::cout << "=== All tests passed! ===\n";
    } catch (const std::exception& e) {
        std::cout << "=== Test failed: " << e.what() << " ===\n";
        return 1;
    } catch (...) {
        std::cout << "=== Test failed: Unknown exception ===\n";
        return 1;
    }

    return 0;
}
