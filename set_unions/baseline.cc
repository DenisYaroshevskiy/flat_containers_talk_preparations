#include "set_unions/common.h"

struct baseline_alg {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return o;
  }
};

void baseline(benchmark::State& state) {
  set_union_bench<baseline_alg>(state);
}

BENCHMARK(baseline)->Apply(set_input_sizes);
