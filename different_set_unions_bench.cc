#include <algorithm>
#include <exception>
#include <random>
#include <set>
#include <vector>

#include "benchmark/benchmark.h"

#include "set_unions.h"

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;

using int_vec = std::vector<int>;

std::pair<int_vec, int_vec> test_input_data(size_t lhs_size, size_t rhs_size) {
  static std::map<std::pair<size_t, size_t>, std::pair<int_vec, int_vec>>
      cached_results;

  auto in_cache = cached_results.find({lhs_size, rhs_size});
  if (in_cache != cached_results.end())
    return in_cache->second;

  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, int(kProblemSize) * 100);
    return dis(g);
  };

  auto generate_vec = [&] (size_t size) {
    std::set<int> res;
    while (res.size() < size)
      res.insert(random_number());
    return int_vec(res.begin(), res.end());
  };

  auto res_and_bool = cached_results.insert(
      {{lhs_size, rhs_size}, {generate_vec(lhs_size), generate_vec(rhs_size)}});
  if (!res_and_bool.second)
    std::terminate();
  return res_and_bool.first->second;
}

template <typename Alg>
void set_union_bench(benchmark::State& state) {
  const size_t lhs_size = static_cast<size_t>(state.range(0));
  const size_t rhs_size = static_cast<size_t>(state.range(1));

  auto input = test_input_data(lhs_size, rhs_size);

  while (state.KeepRunning()) {
    int_vec res(lhs_size + rhs_size);
    Alg{}(input.first.begin(), input.first.end(), input.second.begin(),
          input.second.end(), res.begin());
  }
}

void set_input_sizes(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kMinSize;
  size_t rhs_size = kProblemSize - kMinSize;

  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size += kStep;
    rhs_size -= kStep;
  } while (lhs_size <= kProblemSize);
}

struct baseline_alg {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return o;
  }
};

void baseline(benchmark::State& state) {
  set_union_bench<baseline_alg>(state);
}

struct best_previous_set_union
{
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v5::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void BestPreviousSetUnion(benchmark::State& state) {
  set_union_bench<best_previous_set_union>(state);
}

struct previous_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v7::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void PreviousSetUnion(benchmark::State& state) {
  set_union_bench<previous_set_union>(state);
}

struct current_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v8::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void CurrentSetUnion(benchmark::State& state) {
  set_union_bench<current_set_union>(state);
}

BENCHMARK(baseline)->Apply(set_input_sizes);
BENCHMARK(BestPreviousSetUnion)->Apply(set_input_sizes);
BENCHMARK(PreviousSetUnion)->Apply(set_input_sizes);
BENCHMARK(CurrentSetUnion)->Apply(set_input_sizes);

}  // namespace

BENCHMARK_MAIN();

