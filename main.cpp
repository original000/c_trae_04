#include "IntegerCodec.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <chrono>

void testVarint() {
    std::cout << "Testing Google Varint (LEB128)..." << std::endl;

    // Generate 10,000 random int32 values
    std::vector<int32_t> data;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dis(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());

    for (int i = 0; i < 10000; ++i) {
        data.push_back(dis(gen));
    }

    // Measure encoding time
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> encoded = IntegerCodec::varintEncode(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encode_time = end - start;

    // Measure decoding time
    start = std::chrono::high_resolution_clock::now();
    std::vector<int32_t> decoded = IntegerCodec::varintDecode(encoded);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> decode_time = end - start;

    // Calculate compression ratio
    size_t original_size = data.size() * sizeof(int32_t);
    size_t compressed_size = encoded.size();
    double compression_ratio = IntegerCodec::calculateCompressionRatio(original_size, compressed_size);

    // Verify data integrity
    bool data_valid = true;
    for (size_t i = 0; i < data.size() && i < decoded.size(); ++i) {
        if (data[i] != decoded[i]) {
            data_valid = false;
            break;
        }
    }

    // Print results
    std::cout << "Data size: 10,000 int32 values" << std::endl;
    std::cout << "Original size: " << original_size << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressed_size << " bytes" << std::endl;
    std::cout << "Compression ratio: " << compression_ratio << ":1" << std::endl;
    std::cout << "Encoding time: " << encode_time.count() << " ms" << std::endl;
    std::cout << "Decoding time: " << decode_time.count() << " ms" << std::endl;
    std::cout << "Data integrity: " << (data_valid ? "Valid" : "Invalid") << std::endl;
    std::cout << std::endl;
}

void testGroupVarint() {
    std::cout << "Testing Group Varint (4 integers per group)..." << std::endl;

    // Generate 10,000 random int32 values
    std::vector<int32_t> data;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dis(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());

    for (int i = 0; i < 10000; ++i) {
        data.push_back(dis(gen));
    }

    // Measure encoding time
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> encoded = IntegerCodec::groupVarintEncode(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encode_time = end - start;

    // Measure decoding time
    start = std::chrono::high_resolution_clock::now();
    std::vector<int32_t> decoded = IntegerCodec::groupVarintDecode(encoded);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> decode_time = end - start;

    // Calculate compression ratio
    size_t original_size = data.size() * sizeof(int32_t);
    size_t compressed_size = encoded.size();
    double compression_ratio = IntegerCodec::calculateCompressionRatio(original_size, compressed_size);

    // Verify data integrity (only check up to original size)
    bool data_valid = true;
    for (size_t i = 0; i < data.size() && i < decoded.size(); ++i) {
        if (data[i] != decoded[i]) {
            data_valid = false;
            break;
        }
    }

    // Print results
    std::cout << "Data size: 10,000 int32 values" << std::endl;
    std::cout << "Original size: " << original_size << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressed_size << " bytes" << std::endl;
    std::cout << "Compression ratio: " << compression_ratio << ":1" << std::endl;
    std::cout << "Encoding time: " << encode_time.count() << " ms" << std::endl;
    std::cout << "Decoding time: " << decode_time.count() << " ms" << std::endl;
    std::cout << "Data integrity: " << (data_valid ? "Valid" : "Invalid") << std::endl;
    std::cout << std::endl;
}

void testDeltaEncoding() {
    std::cout << "Testing Delta Encoding..." << std::endl;

    // Generate 10,000 sequential int32 values (delta encoding works best with sequential data)
    std::vector<int32_t> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back(i);
    }

    // Measure encoding time
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int32_t> encoded = IntegerCodec::deltaEncode(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encode_time = end - start;

    // Measure decoding time
    start = std::chrono::high_resolution_clock::now();
    std::vector<int32_t> decoded = IntegerCodec::deltaDecode(encoded);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> decode_time = end - start;

    // Verify data integrity
    bool data_valid = true;
    for (size_t i = 0; i < data.size() && i < decoded.size(); ++i) {
        if (data[i] != decoded[i]) {
            data_valid = false;
            break;
        }
    }

    // Print results
    std::cout << "Data size: 10,000 sequential int32 values" << std::endl;
    std::cout << "Encoding time: " << encode_time.count() << " ms" << std::endl;
    std::cout << "Decoding time: " << decode_time.count() << " ms" << std::endl;
    std::cout << "Data integrity: " << (data_valid ? "Valid" : "Invalid") << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "Integer Compression Codec Benchmark" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;

    testVarint();
    testGroupVarint();
    testDeltaEncoding();

    std::cout << "Benchmark completed!" << std::endl;

    return 0;
}
