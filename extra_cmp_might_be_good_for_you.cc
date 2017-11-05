#include <algorithm>
#include <utility>
#include <iterator>
#include <random>
#include <vector>

#include "benchmark/benchmark.h"

constexpr int kProblemSize = 1000;
constexpr int kDistribution = kProblemSize * 10;

std::pair<std::vector<int>, std::vector<int>>
input_data() {
  auto random_vector = [] {
    std::vector<int> res(kProblemSize);
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, kDistribution * 10);
    std::generate(res.begin(), res.end(), [&] { return dis(g); });
    return res;
  };
  static auto lhs = random_vector();
  static auto rhs = random_vector();
  return std::make_pair(lhs, rhs);
}

template <class I1, class I2, class O, class Comp>
O set_union_lt(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

template <class I1, class I2, class O, class Comp>
O set_union_gt(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (*f1 > *f2) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}


static void extra_cmp(benchmark::State& state) {
  const auto input = input_data();
  std::vector<int> res(kProblemSize * 2);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(set_union_lt(
        input.first.begin(), input.first.end(), input.second.begin(),
        input.second.end(), res.begin(), std::less<>{}));
  }
}

BENCHMARK(extra_cmp);

static void no_extra_cmp(benchmark::State& state) {
   const auto input = input_data();
  std::vector<int> res(kProblemSize * 2);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(set_union_gt(
        input.first.begin(), input.first.end(), input.second.begin(),
        input.second.end(), res.begin(), std::less<>{}));
  }
}

BENCHMARK(no_extra_cmp);

BENCHMARK_MAIN();
