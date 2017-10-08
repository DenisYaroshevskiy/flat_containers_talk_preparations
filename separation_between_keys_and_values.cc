#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "bench_utils.h"
#include "benchmark/benchmark.h"


namespace {

constexpr size_t kSetSize = 100'000;
constexpr int kLookingFor = 700;

template <typename T>
static T& lookup_for_pairs(std::vector<std::pair<int, T>>& cont) {
  return std::lower_bound(cont.begin(), cont.end(), kLookingFor,
                          compare_by_first{})->second;
}

template <typename T>
static T& look_for_mapped_value(const std::vector<int>& keys,
                                std::vector<T>& values) {
  return *(values.begin() +
           (std::lower_bound(keys.begin(), keys.end(), kLookingFor) -
            keys.begin()));
}

template <typename T>
// requires UnsignedIntegral<T>
void single_vector(benchmark::State& state) {
  std::vector<std::pair<int, T>> cont(kSetSize);
  for (int i = 0; i < static_cast<int>(kSetSize); ++i)
    cont[i] = {i, T(0)};

  while (state.KeepRunning())
    ++(lookup_for_pairs(cont));
}

template <typename T>
// requires UnsignedIntegral<T>
void two_vectors(benchmark::State& state) {
  std::vector<int> keys(kSetSize);
  std::vector<T> values(kSetSize);
  std::iota(keys.begin(), keys.end(), 0);

  while (state.KeepRunning())
    ++(look_for_mapped_value(keys, values));
}

void do_nothing(benchmark::State& state) {
  while (state.KeepRunning());
}

}  // namespace

BENCHMARK_TEMPLATE(single_vector, unsigned);
BENCHMARK_TEMPLATE(two_vectors, unsigned);
BENCHMARK_TEMPLATE(single_vector, padded_int<unsigned>);
BENCHMARK_TEMPLATE(two_vectors, padded_int<unsigned>);
BENCHMARK(do_nothing);

BENCHMARK_MAIN();
