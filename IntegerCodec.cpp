#include "IntegerCodec.h"
#include <algorithm>

// Google Varint (LEB128) encoding
std::vector<uint8_t> IntegerCodec::varintEncode(const std::vector<int32_t>& data) {
    std::vector<uint8_t> encoded;
    for (int32_t value : data) {
        uint32_t uvalue = zigZagEncode(value);
        while (true) {
            if ((uvalue & ~0x7F) == 0) {
                encoded.push_back(static_cast<uint8_t>(uvalue));
                break;
            } else {
                encoded.push_back(static_cast<uint8_t>((uvalue & 0x7F) | 0x80));
                uvalue >>= 7;
            }
        }
    }
    return encoded;
}

// Google Varint (LEB128) decoding
std::vector<int32_t> IntegerCodec::varintDecode(const std::vector<uint8_t>& encoded) {
    std::vector<int32_t> decoded;
    size_t i = 0;
    while (i < encoded.size()) {
        uint32_t uvalue = 0;
        int shift = 0;
        uint8_t byte;
        do {
            byte = encoded[i++];
            uvalue |= static_cast<uint32_t>((byte & 0x7F)) << shift;
            shift += 7;
        } while ((byte & 0x80) != 0);
        decoded.push_back(zigZagDecode(uvalue));
    }
    return decoded;
}

// Group Varint encoding (4 integers per group)
std::vector<uint8_t> IntegerCodec::groupVarintEncode(const std::vector<int32_t>& data) {
    std::vector<uint8_t> encoded;
    size_t i = 0;
    size_t n = data.size();

    // Store the original size at the beginning of the encoded data
    encoded.push_back(static_cast<uint8_t>((n >> 0) & 0xFF));
    encoded.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
    encoded.push_back(static_cast<uint8_t>((n >> 24) & 0xFF));

    while (i < n) {
        // Determine the number of bytes needed for each of the 4 integers
        uint8_t control = 0;
        int bytes_needed[4] = {0};
        uint32_t uvalues[4] = {0};

        for (int j = 0; j < 4 && i < n; ++j) {
            uint32_t uvalue = zigZagEncode(data[i++]);
            uvalues[j] = uvalue;

            if (uvalue == 0) {
                bytes_needed[j] = 0;
            } else if (uvalue <= 0xFF) {
                bytes_needed[j] = 1;
            } else if (uvalue <= 0xFFFF) {
                bytes_needed[j] = 2;
            } else if (uvalue <= 0xFFFFFF) {
                bytes_needed[j] = 3;
            } else {
                bytes_needed[j] = 3; // Use 3 to represent 4 bytes
            }

            control |= (bytes_needed[j] << (j * 2));
        }

        encoded.push_back(control);

        // Encode each integer with the determined number of bytes
        for (int j = 0; j < 4 && (i - 4 + j) < n; ++j) {
            int bytes = bytes_needed[j];
            if (bytes == 3) {
                bytes = 4; // 3 in bytes_needed represents 4 bytes
            }
            uint32_t uvalue = uvalues[j];

            for (int k = 0; k < bytes; ++k) {
                encoded.push_back(static_cast<uint8_t>(uvalue & 0xFF));
                uvalue >>= 8;
            }
        }
    }

    return encoded;
}

// Group Varint decoding (4 integers per group)
std::vector<int32_t> IntegerCodec::groupVarintDecode(const std::vector<uint8_t>& encoded) {
    std::vector<int32_t> decoded;
    if (encoded.size() < 4) {
        return decoded; // Not enough data to even read the original size
    }

    // Read the original size from the beginning of the encoded data
    size_t original_size = static_cast<size_t>(encoded[0]) | 
                            (static_cast<size_t>(encoded[1]) << 8) | 
                            (static_cast<size_t>(encoded[2]) << 16) | 
                            (static_cast<size_t>(encoded[3]) << 24);

    size_t i = 4; // Skip the first 4 bytes which contain the original size
    size_t n = encoded.size();

    while (i < n && decoded.size() < original_size) {
        uint8_t control = encoded[i++];

        for (int j = 0; j < 4 && decoded.size() < original_size; ++j) {
            int bytes = (control >> (j * 2)) & 0x03;
            if (bytes == 3) {
                bytes = 4; // 3 in control byte represents 4 bytes
            }

            if (bytes == 0) {
                decoded.push_back(0);
            } else {
                uint32_t uvalue = 0;
                for (int k = 0; k < bytes && i < n; ++k) {
                    uvalue |= static_cast<uint32_t>(encoded[i++]) << (k * 8);
                }
                decoded.push_back(zigZagDecode(uvalue));
            }
        }
    }

    return decoded;
}

// Delta encoding
std::vector<int32_t> IntegerCodec::deltaEncode(const std::vector<int32_t>& data) {
    std::vector<int32_t> encoded;
    if (data.empty()) return encoded;

    encoded.push_back(data[0]);
    for (size_t i = 1; i < data.size(); ++i) {
        encoded.push_back(data[i] - data[i-1]);
    }

    return encoded;
}

// Delta decoding
std::vector<int32_t> IntegerCodec::deltaDecode(const std::vector<int32_t>& encoded) {
    std::vector<int32_t> decoded;
    if (encoded.empty()) return decoded;

    decoded.push_back(encoded[0]);
    for (size_t i = 1; i < encoded.size(); ++i) {
        decoded.push_back(decoded[i-1] + encoded[i]);
    }

    return decoded;
}

// ZigZag encoding (signed to unsigned conversion)
uint32_t IntegerCodec::zigZagEncode(int32_t value) {
    return static_cast<uint32_t>((value << 1) ^ (value >> 31));
}

// ZigZag decoding (unsigned to signed conversion)
int32_t IntegerCodec::zigZagDecode(uint32_t value) {
    return static_cast<int32_t>((value >> 1) ^ -(static_cast<int32_t>(value & 1)));
}

// Compression ratio calculation
double IntegerCodec::calculateCompressionRatio(size_t originalSize, size_t compressedSize) {
    if (originalSize == 0) return 0.0;
    return static_cast<double>(originalSize) / static_cast<double>(compressedSize);
}
