#include <algorithm>
#include <random>
#include <map>

#include "lib.h"
//#include "linear_merge_boundary.h"
#include "base/containers/flat_set.h"
#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kLhsSize = 1000;

constexpr size_t kStartRhs = 1;
constexpr size_t kEndRhs = 31;
constexpr size_t kRhsStep = 1;

using int_vec = std::vector<int>;

std::pair<int_vec*, int_vec*> test_input_data(int inserting_size) {
  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, int(kLhsSize) * 100);
    return dis(g);
  };

  static int_vec already_in = [&] {
    std::set<int> res;
    while (res.size() < kLhsSize)
      res.insert(random_number());
    return int_vec(res.begin(), res.end());
  }();

  static std::map<int, std::vector<int>> inserting_cache;

  auto found = inserting_cache.find(inserting_size);
  if (found == inserting_cache.end()) {
    found = inserting_cache.insert({inserting_size,
                                    [&] {
                                      int_vec res(
                                          static_cast<size_t>(inserting_size));
                                      std::generate(res.begin(), res.end(),
                                                    random_number);
                                      return res;
                                    }()}).first;
  }

  return {&already_in, &found->second};
}

template <typename Container>
void insert_first_last_bench(benchmark::State& state) {
  auto input = test_input_data(state.range(0));
  Container c(input.first->begin(), input.first->end());
  while (state.KeepRunning()) {
    auto copy = c;
    copy.insert(input.second->begin(), input.second->end());
  }
}

struct baseline_set {
  template <typename I>
  baseline_set(I f, I l)
      : body(f, l){};

  template <typename I>
  void insert(I f, I l) {}

  int_vec body;
};

void set_input_sizes(benchmark::internal::Benchmark* bench) {
  for (int i = static_cast<int>(kStartRhs); i < static_cast<int>(kEndRhs);
       i += static_cast<int>(kRhsStep))
    bench->Arg(i);
}

void baseline(benchmark::State& state) {
  insert_first_last_bench<baseline_set>(state);
}
BENCHMARK(baseline)->Apply(set_input_sizes);

void UseMemmove(benchmark::State& state) {
  insert_first_last_bench<lib::flat_set<int>>(state);
}
BENCHMARK(UseMemmove)->Apply(set_input_sizes);

void Boost(benchmark::State& state) {
  insert_first_last_bench<boost::container::flat_set<int>>(state);
}
BENCHMARK(Boost)->Apply(set_input_sizes);

#if 0
void Folly(benchmark::State& state) {
  insert_first_last_bench<folly::sorted_vector_set<int>>(state);
}
BENCHMARK(Folly)->Apply(set_input_sizes);
#endif

}  // namespace

BENCHMARK_MAIN();
