#include <algorithm>
#include <random>

#include "lib.h"
#include "base/containers/flat_set.h"
#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kLhsSize = 1000;
constexpr size_t kRhsSize = 25;

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

template <typename Container>
void insert_first_last_bench(benchmark::State& state) {
  auto input = generate_data();
  Container c(input.first.begin(), input.first.end());
  while (state.KeepRunning()) {
    auto copy = c;
    copy.insert(input.second.begin(), input.second.end());
  }
}

void insert_first_last_bench_lib(benchmark::State& state) {
  insert_first_last_bench<lib::flat_set<int>>(state);
}

void insert_first_last_bench_boost(benchmark::State& state) {
  insert_first_last_bench<boost::container::flat_set<int>>(state);
}

void insert_first_last_bench_folly(benchmark::State& state) {
  insert_first_last_bench<folly::sorted_vector_set<int>>(state);
}

}  // namespace

BENCHMARK(insert_first_last_bench_lib);
BENCHMARK(insert_first_last_bench_boost);
BENCHMARK(insert_first_last_bench_folly);

BENCHMARK_MAIN();


