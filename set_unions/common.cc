#include "set_unions/common.h"

#include <algorithm>
#include <exception>
#include <random>
#include <set>

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;
constexpr bool kLastStep = false;

void full_problem_size(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kMinSize;
  size_t rhs_size = kProblemSize - kMinSize;

  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size += kStep;
    rhs_size -= kStep;
  } while (lhs_size <= kProblemSize);
}

void last_step(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kProblemSize - kStep;
  size_t rhs_size = kStep;
  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size += 1;
    rhs_size -= 1;
  } while (lhs_size <= kProblemSize);
}

}  // namespace

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

  auto generate_vec = [&](size_t size) {
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

void set_input_sizes(benchmark::internal::Benchmark* bench) {
#if 0
  bench->Args({560, 1440});
  return;
#endif
  if (kLastStep) {
    last_step(bench);
  } else {
    full_problem_size(bench);
  }
}

BENCHMARK_MAIN();
