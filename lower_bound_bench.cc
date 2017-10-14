#include <algorithm>
#include <numeric>

#include "lib.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSetSize = 10000;
constexpr int kLookingFor = 100;

template <typename F>
void lower_bound_alg(benchmark::State& state) {
  std::vector<int> cont(kSetSize);
  std::iota(cont.begin(), cont.end(), 0);
  F f;

  while (state.KeepRunning())
    benchmark::DoNotOptimize(f(cont.begin(), cont.end(), kLookingFor));
}


struct linear_search {
  template <typename I, typename T>
  I operator()(I f, I l, const T& v) {
    return lib::lower_bound_linear(f, l, v);
  }
};

struct simple_biased {
  template <typename I, typename T>
  I operator()(I f, I l, const T& v) {
    return lib::lower_bound_biased(f, l, v);
  }
};

struct sentinal_biased {
  template <typename I, typename T>
  I operator()(I f, I l, const T& v) {
    return lib::lower_bound_biased_sentinal(f, l, v);
  }
};

struct binary_search {
  template <typename I, typename T>
  I operator()(I f, I l, const T& v) {
    return std::lower_bound(f, l, v);
  }
};

}  // namespace

BENCHMARK_TEMPLATE(lower_bound_alg, linear_search);
BENCHMARK_TEMPLATE(lower_bound_alg, simple_biased);
BENCHMARK_TEMPLATE(lower_bound_alg, sentinal_biased);
BENCHMARK_TEMPLATE(lower_bound_alg, binary_search);

BENCHMARK_MAIN();

