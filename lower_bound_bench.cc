#include <algorithm>
#include <numeric>

#include "lib.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSetSize = 1000;
constexpr int kLookingFor = 400;

template <typename I, typename P>
// requires Input<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_linear(I f, I l, P p) {
  return std::find_if_not(f, l, p);
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_linear(I f, I l, const V& v, P p) {
  return partition_point_linear(f, l, lib::less_than(v, p));
}

template <typename I, typename V>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_linear(I f, I l, const V& v) {
  return lower_bound_linear(f, l, v, lib::less{});
}

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased_simple(I f, I l, P p) {
  if (f == l || !p(*f))
    return f;
  ++f;
  auto len = std::distance(f, l);
  auto step = 1;
  while (len > step) {
    I m = std::next(f, step);
    if (!p(*m)) {
      l = m;
      break;
    }
    f = ++m;
    len -= step + 1;
    step <<= 1;
  }
  return std::partition_point(f, l, p);
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_biased_simple(I f, I l, const V& v, P p) {
  return partition_point_biased_simple(f, l, lib::less_than(v, p));
}

template <typename I, typename V>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_biased_simple(I f, I l, const V& v) {
  return lower_bound_biased_simple(f, l, v, lib::less{});
}

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
    return lower_bound_linear(f, l, v);
  }
};

struct simple_biased {
  template <typename I, typename T>
  I operator()(I f, I l, const T& v) {
    return lower_bound_biased_simple(f, l, v);
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
