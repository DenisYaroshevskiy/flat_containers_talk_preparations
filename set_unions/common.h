#include <utility>

#include "benchmark/benchmark.h"

using int_vec = std::vector<int>;

void set_input_sizes(benchmark::internal::Benchmark* bench);

std::pair<int_vec, int_vec> test_input_data(size_t lhs_size, size_t rhs_size);

template <typename Alg>
void set_union_bench(benchmark::State& state) {
  const size_t lhs_size = static_cast<size_t>(state.range(0));
  const size_t rhs_size = static_cast<size_t>(state.range(1));

  auto input = test_input_data(lhs_size, rhs_size);

  for (auto _ : state) {
    int_vec res(lhs_size + rhs_size);
    Alg{}(input.first.begin(), input.first.end(), input.second.begin(),
          input.second.end(), res.begin());
  }
}
