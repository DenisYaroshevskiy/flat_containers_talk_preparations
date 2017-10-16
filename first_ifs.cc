#include <algorithm>
#include <array>
#include <vector>
#include <random>

#include "benchmark/benchmark.h"

namespace {

using value_type = std::array<int, 15>;
constexpr size_t kArraySize = 1000;
constexpr size_t kLookingFor = 200;

std::vector<value_type> generate_input() {
  std::mt19937 g;
  std::uniform_int_distribution<> dis(1, 10000);

  std::vector<value_type> res(kArraySize);


  for (auto& x : res) {
    std::generate(x.begin(), x.end(), [] {
      return dis(g);
    });
  }

  std::sort(res.begin(), res.end());
  return res;
};

void lower_bound_first_element(benchmark::State& state) {
  const auto v = generate_input();
  const auto looking_for = v[kLookingFor];

  while (state.KeepRunning())
    benchmark::DoNotOptimize(std::lower_bound(
        v.begin(), v.end(), looking_for,
        [](const auto& x, const auto& y) { return x[0] < y[0]; }));
}

void lower_bound_full_compare(benchmark::State& state) {
  const auto v = generate_input();
  const auto looking_for = v[kLookingFor];
  while (state.KeepRunning())
    benchmark::DoNotOptimize(std::lower_bound(v.begin(), v.end(), looking_for));
}

BENCHMARK(lower_bound_first_element);
BENCHMARK(lower_bound_full_compare);
}

BENCHMARK_MAIN();
