#ifndef ALPHABET_FORGE_BENCH_H
#define ALPHABET_FORGE_BENCH_H

#include "forge_spec.h"
#include <string>
#include <vector>

namespace alphabet {
namespace forge {

struct BenchResult {
    int iterations = 0;
    double min_ms = 0.0;
    double max_ms = 0.0;
    double avg_ms = 0.0;
    double total_ms = 0.0;
    double ops_per_sec = 0.0;
    std::vector<double> runs;
};

class ForgeBenchmark {
public:
    // Execute benchmark across multiple iterations and print statistical card
    static BenchResult run(const std::string& filepath, const ForgeSpec& spec, int iterations = 5);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_BENCH_H
