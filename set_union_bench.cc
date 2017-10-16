#include <algorithm>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include "lib.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kLhsSize = 1000;
constexpr size_t kRhsSize = 50;

using int_vec = std::vector<int>;

std::pair<int_vec, int_vec> generate_data() {
  std::mt19937 g;
  std::uniform_int_distribution<> dis(1, 10000);

  auto rand_int = [&] { return dis(g); };

  int_vec lhs(kLhsSize);
  int_vec rhs(kRhsSize);
  std::generate(lhs.begin(), lhs.end(), rand_int);
  lhs.erase(lib::sort_and_unique(lhs.begin(), lhs.end()), lhs.end());

  std::generate(rhs.begin(), rhs.end(), rand_int);
  rhs.erase(lib::sort_and_unique(rhs.begin(), rhs.end()), rhs.end());

  return {std::move(lhs), std::move(rhs)};
}

struct std_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return std::set_union(f1, l1, f2, l2, o);
  }
};

struct set_union_unbalanced {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return lib::set_union_unbalanced(f1, l1, f2, l2, o);
  }
};

template <typename F>
void set_union_benchmark(benchmark::State& state) {
  F f;
  auto input = generate_data();
  int_vec out(kLhsSize + kRhsSize);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(f(input.first.begin(), input.first.end(),
                               input.second.begin(), input.second.end(),
                               out.begin()));
  }
}

BENCHMARK_TEMPLATE(set_union_benchmark, std_set_union);
BENCHMARK_TEMPLATE(set_union_benchmark, set_union_unbalanced);


}  // namespace

BENCHMARK_MAIN();

