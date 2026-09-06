#include "forge_bench.h"
#include "forge.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <algorithm>

namespace alphabet {
namespace forge {

BenchResult ForgeBenchmark::run(const std::string& filepath, const ForgeSpec& spec, int iterations) {
    BenchResult res;
    res.iterations = std::max(1, iterations);

    std::cout << "\n\033[1mBenchmarking " << spec.name << " Execution: " << filepath << "\033[0m\n";
    std::cout << "Iterations: " << res.iterations << "\n";
    std::cout << "──────────────────────────────────────────────────────\n";

    for (int i = 0; i < res.iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        // Run program silently (or with standard run)
        bool ok = ForgeRunner::run_file(filepath, spec.spec_source_path, false);
        (void)ok;

        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        res.runs.push_back(ms);
        res.total_ms += ms;

        std::cout << "  Run #" << (i + 1) << ": " 
                  << std::fixed << std::setprecision(2) << ms << " ms\n";
    }

    if (!res.runs.empty()) {
        res.min_ms = *std::min_element(res.runs.begin(), res.runs.end());
        res.max_ms = *std::max_element(res.runs.begin(), res.runs.end());
        res.avg_ms = res.total_ms / res.runs.size();
        res.ops_per_sec = (res.avg_ms > 0.0) ? (1000.0 / res.avg_ms) : 0.0;
    }

    std::cout << "──────────────────────────────────────────────────────\n";
    std::cout << "\033[1;36mBenchmark Results:\033[0m\n";
    std::cout << "  • \033[1mFastest:\033[0m    " << std::fixed << std::setprecision(2) << res.min_ms << " ms\n";
    std::cout << "  • \033[1mSlowest:\033[0m    " << std::fixed << std::setprecision(2) << res.max_ms << " ms\n";
    std::cout << "  • \033[1mAverage:\033[0m    " << std::fixed << std::setprecision(2) << res.avg_ms << " ms\n";
    std::cout << "  • \033[1mThroughput:\033[0m " << std::fixed << std::setprecision(1) << res.ops_per_sec << " runs/sec\n\n";

    return res;
}

} // namespace forge
} // namespace alphabet
