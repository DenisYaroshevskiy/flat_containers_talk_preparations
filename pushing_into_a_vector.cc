#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

#include "benchmark/benchmark.h"

constexpr size_t kSize = 1000;

void insert_f_l(benchmark::State& state) {
  std::vector<int> in(kSize);
  std::iota(in.begin(), in.end(), 0);

  while(state.KeepRunning()) {
    std::vector<int> v;
    benchmark::DoNotOptimize(v.insert(v.cend(), in.begin(), in.end()));
  }
}

void back_inserter(benchmark::State& state) {
  std::vector<int> in(kSize);
  std::iota(in.begin(), in.end(), 0);

  while(state.KeepRunning()) {
    std::vector<int> v;
    benchmark::DoNotOptimize(
        std::copy(in.begin(), in.end(), std::back_inserter(v)));
  }
}

BENCHMARK(insert_f_l);
BENCHMARK(back_inserter);
BENCHMARK_MAIN();
