#include "lib.h"

#include <functional>

#define CATCH_CONFIG_MAIN
#include "third_party/catch.h"

namespace {

struct strange_cmp : lib::less {
  explicit strange_cmp(int) {}
};

using int_set = lib::flat_set<int>;
using int_vec = int_set::underlying_type;
using strange_cmp_set = lib::flat_set<int, strange_cmp>;

}  // namespace

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
