#ifndef INTEGERCODEC_H
#define INTEGERCODEC_H

#include <vector>
#include <cstdint>

class IntegerCodec {
public:
    // Google Varint (LEB128) encoding/decoding
    static std::vector<uint8_t> varintEncode(const std::vector<int32_t>& data);
    static std::vector<int32_t> varintDecode(const std::vector<uint8_t>& encoded);

    // Group Varint encoding/decoding (4 integers per group)
    static std::vector<uint8_t> groupVarintEncode(const std::vector<int32_t>& data);
    static std::vector<int32_t> groupVarintDecode(const std::vector<uint8_t>& encoded);

    // Delta encoding/decoding
    static std::vector<int32_t> deltaEncode(const std::vector<int32_t>& data);
    static std::vector<int32_t> deltaDecode(const std::vector<int32_t>& encoded);

    // ZigZag encoding/decoding (signed to unsigned conversion)
    static uint32_t zigZagEncode(int32_t value);
    static int32_t zigZagDecode(uint32_t value);

    // Compression ratio calculation
    static double calculateCompressionRatio(size_t originalSize, size_t compressedSize);
};

#endif // INTEGERCODEC_H