#include "VectorDB.h"
#include <algorithm>
#include <numeric>
#include <cmath>

VectorDB::VectorDB(SimilarityType type)
    : similarity_type_(type)
{
}

void VectorDB::insert(const float* vector, int64_t id)
{
    Vector vec;
    std::copy(vector, vector + DIM, vec.data);
    vec.id = id;
    vectors_.push_back(vec);
}

std::vector<int64_t> VectorDB::search_topk(const float* query, int k) const
{
    std::vector<std::pair<float, int64_t>> scores;
    scores.reserve(vectors_.size());

    for (const auto& vec : vectors_) {
        // Prefetch the next vector to improve cache performance
        __builtin_prefetch(&vectors_[&vec - &vectors_[0] + 1], 0, 3);

        float score = compute_similarity(query, vec.data);
        scores.emplace_back(score, vec.id);
    }

    // Sort scores in descending order to get top k
    std::partial_sort(scores.begin(), scores.begin() + k, scores.end(),
        [](const std::pair<float, int64_t>& a, const std::pair<float, int64_t>& b) {
            return a.first > b.first;
        });

    std::vector<int64_t> result;
    result.reserve(k);
    for (int i = 0; i < k; ++i) {
        result.push_back(scores[i].second);
    }

    return result;
}

float VectorDB::compute_similarity(const float* a, const float* b) const
{
    if (similarity_type_ == SimilarityType::INNER_PRODUCT) {
        return compute_inner_product(a, b);
    } else {
        return compute_cosine_similarity(a, b);
    }
}

float VectorDB::compute_inner_product(const float* a, const float* b) const
{
    float sum = 0.0f;

    // Manual loop unrolling for better performance
    int i = 0;
    for (; i < DIM - 7; i += 8) {
        sum += a[i] * b[i];
        sum += a[i + 1] * b[i + 1];
        sum += a[i + 2] * b[i + 2];
        sum += a[i + 3] * b[i + 3];
        sum += a[i + 4] * b[i + 4];
        sum += a[i + 5] * b[i + 5];
        sum += a[i + 6] * b[i + 6];
        sum += a[i + 7] * b[i + 7];
    }

    // Handle remaining elements
    for (; i < DIM; ++i) {
        sum += a[i] * b[i];
    }

    return sum;
}

float VectorDB::compute_cosine_similarity(const float* a, const float* b) const
{
    float dot_product = compute_inner_product(a, b);
    float norm_a = compute_norm(a);
    float norm_b = compute_norm(b);

    if (norm_a == 0.0f || norm_b == 0.0f) {
        return 0.0f;
    }

    return dot_product / (norm_a * norm_b);
}

float VectorDB::compute_norm(const float* vector) const
{
    float sum_sq = 0.0f;

    // Manual loop unrolling for better performance
    int i = 0;
    for (; i < DIM - 7; i += 8) {
        sum_sq += vector[i] * vector[i];
        sum_sq += vector[i + 1] * vector[i + 1];
        sum_sq += vector[i + 2] * vector[i + 2];
        sum_sq += vector[i + 3] * vector[i + 3];
        sum_sq += vector[i + 4] * vector[i + 4];
        sum_sq += vector[i + 5] * vector[i + 5];
        sum_sq += vector[i + 6] * vector[i + 6];
        sum_sq += vector[i + 7] * vector[i + 7];
    }

    // Handle remaining elements
    for (; i < DIM; ++i) {
        sum_sq += vector[i] * vector[i];
    }

    return std::sqrt(sum_sq);
}
