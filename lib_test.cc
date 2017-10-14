
#define LIB_TEST_ON
#define CATCH_CONFIG_MAIN
#include "third_party/catch.h"

#include "lib.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

#include <iostream>


namespace {

struct strange_cmp : lib::less {
  explicit strange_cmp(int) {}
};

using int_set = lib::flat_set<int>;
using int_vec = int_set::underlying_type;
using strange_cmp_set = lib::flat_set<int, strange_cmp>;

template <typename F>
void lower_bound_test(F f) {
  for (size_t size = 6; size < 1000; ++size) {
    std::cout << size << std::endl;
    std::vector<int> v(size);
    std::iota(v.begin(), v.end(), 0);
    for (int looking_for : v) {
      // std::cout << "  " << looking_for << std::endl;
      auto expected = std::lower_bound(v.begin(), v.end(), looking_for);
      auto actual = f(v, looking_for);
      REQUIRE(expected == actual);
    }
  }
}

}  // namespace

TEST_CASE("lower_bound_biased_old", "[partition_point_biased]") {
  lower_bound_test([](const std::vector<int>& v, int looking_for) {
    return lib::lower_bound_biased(v.begin(), v.end(), looking_for);
  });
}

TEST_CASE("lower_bound_biased_sentinal", "[partition_point_biased]") {
  lower_bound_test([](const std::vector<int>& v, int looking_for) {
    return lib::lower_bound_biased_sentinal(v.begin(), v.end(), looking_for);
  });
}

TEST_CASE("set_types", "[flat_containers, flat_set]") {
  // These are guaranteed to be portable.
  static_assert((std::is_same<int, int_set::key_type>::value), "");
  static_assert((std::is_same<int, int_set::value_type>::value), "");
  static_assert((std::is_same<lib::less, int_set::key_compare>::value), "");
  static_assert((std::is_same<int&, int_set::reference>::value), "");
  static_assert((std::is_same<const int&, int_set::const_reference>::value),
                "");
  static_assert((std::is_same<int*, int_set::pointer>::value), "");
  static_assert((std::is_same<const int*, int_set::const_pointer>::value), "");

  static_assert(sizeof(int_set) == sizeof(int_set::underlying_type), "");
}

TEST_CASE("incomplete_type", "[flat_containers, flat_set]") {
  struct t {
    using dependent = lib::flat_set<t>;
    int data;
    dependent set_with_incomplete_type;
    dependent::iterator it;
    dependent::const_iterator cit;

    // We do not declare operator< because clang complains that it's unused.
  };

  t x;
}

TEST_CASE("default_constructor", "[flat_containers, flat_set]") {
  {
    const int_set c;
    REQUIRE(c.body() == int_vec());
  }
  {
    const strange_cmp_set c(strange_cmp(0));
    REQUIRE(c.body() == int_vec());
  }
}

TEST_CASE("range_constructor", "[flat_containers, flat_set]") {
  {
    const int_set c(int_set({1, 1, 2, 2, 1, 2, 3, 3, 3}));
    const int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
  {
    const int_set c{1, 1, 2, 2, 1, 2, 3, 3, 3};
    const int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
  {
    const int_vec input{1, 1, 2, 2, 1, 2, 3, 3, 3};
    const int_set c(input.begin(), input.end());
    const int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
}

TEST_CASE("copy_constructor", "[flat_containers, flat_set]") {
  int_set original{1, 2, 3, 4};
  auto copy(original);

  REQUIRE(copy == original);
}
