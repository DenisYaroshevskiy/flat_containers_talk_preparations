#include <algorithm>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "base/containers/flat_set.h"
#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>
#include "lib.h"

#include "benchmark/benchmark.h"

using value_type = int;
constexpr size_t kSize = 1000;

namespace {

std::vector<value_type> generate_input() {
  static auto res = [] {
    std::mt19937 g;
    std::uniform_int_distribution<> dis(1, 10000);

    std::vector<value_type> v(kSize);
    std::generate(v.begin(), v.end(), [&] { return dis(g); });
    return v;
  }();

  return res;
}

template <typename contaier>
void range_construction(benchmark::State& state) {
  std::vector<value_type> v = generate_input();

  while(state.KeepRunning())
    benchmark::DoNotOptimize(contaier(v.begin(), v.end()));
}

}  // namespace

BENCHMARK_TEMPLATE(range_construction, lib::flat_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, folly::sorted_vector_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, base::flat_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, boost::container::flat_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, std::unordered_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, std::set<value_type>);

BENCHMARK_MAIN();
