#include "VectorDB.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    // Create VectorDB with cosine similarity
    VectorDB vdb(SimilarityType::COSINE_SIMILARITY);

    // Insert 500 random 128-dimensional vectors
    const int num_vectors = 500;
    float vector[DIM];

    for (int i = 0; i < num_vectors; ++i) {
        // Generate random vector
        for (int j = 0; j < DIM; ++j) {
            vector[j] = dis(gen);
        }
        // Insert into VectorDB
        vdb.insert(vector, i);
    }

    // Perform 100 Top-10 queries and measure average time
    const int num_queries = 100;
    const int k = 10;
    float query[DIM];
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::milli> total_time(0);

    for (int i = 0; i < num_queries; ++i) {
        // Generate random query vector
        for (int j = 0; j < DIM; ++j) {
            query[j] = dis(gen);
        }

        // Perform search and measure time
        start = std::chrono::high_resolution_clock::now();
        std::vector<int64_t> result = vdb.search_topk(query, k);
        end = std::chrono::high_resolution_clock::now();

        // Accumulate total time
        total_time += std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);

        // Print first query result (for verification)
        if (i == 0) {
            std::cout << "First query result (Top-10 IDs):" << std::endl;
            for (int64_t id : result) {
                std::cout << id << " ";
            }
            std::cout << std::endl;
        }
    }

    // Calculate and print average time per query
    double average_time = total_time.count() / num_queries;
    std::cout << "Average time per Top-10 query: " << average_time << " ms" << std::endl;

    return 0;
}
