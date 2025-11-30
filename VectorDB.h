#pragma once

#include <cstdint>
#include <vector>
#include <memory>

constexpr int DIM = 128;

enum class SimilarityType {
    INNER_PRODUCT,
    COSINE_SIMILARITY
};

struct Vector {
    float data[DIM];
    int64_t id;
};

class VectorDB {
public:
    VectorDB(SimilarityType type = SimilarityType::INNER_PRODUCT);
    ~VectorDB() = default;

    void insert(const float* vector, int64_t id);
    std::vector<int64_t> search_topk(const float* query, int k) const;

private:
    SimilarityType similarity_type_;
    std::vector<Vector> vectors_;

    float compute_similarity(const float* a, const float* b) const;
    float compute_inner_product(const float* a, const float* b) const;
    float compute_cosine_similarity(const float* a, const float* b) const;
    float compute_norm(const float* vector) const;
};
