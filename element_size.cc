#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "bench_utils.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSetSize = 1000;
constexpr int kLookingFor = 700;

template <typename T>
// requires Integral<T>
void lower_bound_bench(benchmark::State& state) {
  std::vector<T> cont(kSetSize);
  std::iota(cont.begin(), cont.end(), 0);

  while (state.KeepRunning())
    benchmark::DoNotOptimize(
        std::lower_bound(cont.begin(), cont.end(), kLookingFor));
}

template <typename T>
void copy_bench(benchmark::State& state) {
  std::vector<T> cont(kSetSize);
  std::iota(cont.begin(), cont.end(), 0);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(std::vector<T>(cont));
  }
}

template <typename T>
void erase_if_bench(benchmark::State& state) {
  std::vector<T> cont(kSetSize);
  std::iota(cont.begin(), cont.end(), 0);

  while (state.KeepRunning()) {
    auto duplicate = cont;
    duplicate.erase(std::remove_if(duplicate.begin(), duplicate.end(),
                                   [](const T& x) { return x % 2; }));
  }
}

void do_nothing(benchmark::State& state) {
  while (state.KeepRunning())
    ;
}

}  // namespace

BENCHMARK_TEMPLATE(lower_bound_bench, std::int32_t);
BENCHMARK_TEMPLATE(lower_bound_bench, padded_int<std::int32_t>);
BENCHMARK_TEMPLATE(copy_bench, std::int32_t);
BENCHMARK_TEMPLATE(copy_bench, padded_int<std::int32_t>);
BENCHMARK_TEMPLATE(erase_if_bench, std::int32_t);
BENCHMARK_TEMPLATE(erase_if_bench, padded_int<std::int32_t>);
BENCHMARK(do_nothing);

BENCHMARK_MAIN();
