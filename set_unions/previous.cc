#include "set_unions.h"
#include "set_unions/common.h"

struct previous_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v7::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void PreviousSetUnion(benchmark::State& state) {
  set_union_bench<previous_set_union>(state);
}

BENCHMARK(PreviousSetUnion)->Apply(set_input_sizes);
