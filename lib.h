#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

namespace lib {

namespace detail {

template <typename...>
using void_t = void;

template <typename, typename = void>
struct has_is_transparent_member : std::false_type {};

template <typename T>
struct has_is_transparent_member<T, void_t<typename T::is_transparent>>
    : std::true_type {};

template <typename F>
// requires Predicate<F>
struct not_fn_impl {
  F f;

  // clang-format off
  template <typename... Args>
  bool operator()(Args&&... args)
  noexcept(
    noexcept(f(std::forward<Args>(args)...))
  ) {
    return !f(std::forward<Args>(args)...);
  }
  // clang-format on
};

template <typename F, typename T>
// requires Ordering<F(T)>
struct less_than_t : F {
  const T* x;

  less_than_t(F f, const T& x) noexcept : F{f}, x(&x) {}

  // clang-format off
  template <typename U>
  bool operator()(const U& y)
  noexcept (
    noexcept(f(std::forward<U>(y)))
  ) {
    return F::operator()(y, *x);
  }
  // clang-format on

};

template <typename F>
// requires BinaryPredicate<F(T)>
struct strict_opposite {
  F f;

  // clang-format off
  template <typename T, typename U>
  bool operator()(T&& x, U&& y)
  noexcept(
    noexcept(f(std::forward<U>(y), std::forward<T>(x)))
  ) {
    return f(std::forward<U>(y), std::forward<T>(x));
  }
  // clang-format on
};

}  // namespace detail

// concepts -------------------------------------------------------------------

template <typename T>
constexpr bool TransparentComparator() {
  return detail::has_is_transparent_member<T>::value;
}

// functors -------------------------------------------------------------------

struct less {
  template <typename X, typename Y>
  bool operator()(const X& x, const Y& y) noexcept(noexcept(x < y)) {
    return x < y;
  }

  using is_transparent = int;
};

template <typename F>
// requires Predicate<F>
detail::not_fn_impl<F> not_fn(F f) noexcept {
  return {f};
}

template <typename F, typename T>
// requires Ordering<F(T)>
detail::less_than_t<F, T> less_than(const T& x, F f) noexcept {
  return {f, x};
}

template <typename T>
// requires Ordered<T>
auto less_than(const T& x) noexcept -> decltype(less_than(x, less{})) {
  return less_than(x, less{});
}

// algorithms -----------------------------------------------------------------

// Think: stable_sort is a merge sort. Merge can be replaced with set_union ->
// unique would not be required. Quick sort is not modified that easily (is it?)
// to do this. How much does the unique matter? For the 1000 elements - log is
// 10 - unique is 1 => 1/10? Measuring this would be cool.

template <typename I, typename Comparator>
// requires RandomAccessIterator<I>() && // It's possible to use Forward
//                                       // but I would have to redo std::sort.
//          StrictWeakOrdering<Comparator(ValueType<I>())>
I sort_and_unique(I f, I l, Comparator comp) {
  std::sort(f, l, comp);
  return std::unique(f, l, not_fn(comp));
}

template <typename I>
I sort_and_unique(I f, I l) {
  return sort_and_unique(f, l, less{});
}

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
  auto n = std::distance(f, l);
  if (n <= 5)
    return std::find_if_not(f, l, p);

  if (!p(*f))
    return f;
  ++f;
  --n;

  I middle = std::next(f, n / 2);
  if (p(*middle))
    return std::partition_point(++middle, l, p);

  int step = 1;

  while (true) {
    I test = std::next(f, step);
    if (!p(*test)) {
      l = test;
      break;
    }
    f = ++test;
    step <<= 1;
  }

  return std::partition_point(f, std::min(l, middle), p);
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_biased(I f, I l, const V& v, P p) {
  return partition_point_biased(f, l, less_than(v, p));
}

template <typename I, typename V>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_biased(I f, I l, const V& v) {
  return lower_bound_biased(f, l, v, less{});
}

template <typename I, typename V>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I upper_bound_biased(I f, I l, const V& v, P p) {
  return partition_point_biased(f, l, )
}

template <typename Key,
          typename Comparator = less,
          typename UnderlyingType = std::vector<Key>>
// requires (todo)
class flat_set {
 public:
  using underlying_type = UnderlyingType;
  using key_type = Key;
  using value_type = key_type;
  using size_type = typename underlying_type::size_type;
  using difference_type = typename underlying_type::difference_type;
  using key_compare = Comparator;
  using value_compare = Comparator;
  using reference = typename underlying_type::reference;
  using const_reference = typename underlying_type::const_reference;
  using pointer = typename underlying_type::pointer;
  using const_pointer = typename underlying_type::const_pointer;
  using iterator = typename underlying_type::iterator;
  using const_iterator = typename underlying_type::const_iterator;
  using reverse_iterator = typename underlying_type::reverse_iterator;
  using const_reverse_iterator =
      typename underlying_type::const_reverse_iterator;

 private:
  // tuple compresses one empty argument
  std::tuple<underlying_type, value_compare> impl_;

 public:
  flat_set() = default;
  flat_set(const flat_set&) = default;
  flat_set(flat_set&&) = default;
  flat_set& operator=(const flat_set&) = default;
  flat_set& operator=(flat_set&&) = default;
  ~flat_set() = default;

  // clang-format off
  explicit flat_set(const key_compare& comp)
    noexcept(
      std::is_nothrow_default_constructible<underlying_type>::value &&
      std::is_nothrow_copy_constructible<key_compare>::value
    )
    : impl_{{}, comp} {}
  // clang-format on

  // clang-format off
  flat_set(underlying_type buf, const key_compare& comp = key_compare())
    noexcept(
      std::is_nothrow_move_constructible<underlying_type>::value &&
      std::is_nothrow_move_constructible<key_compare>::value &&
      std::is_nothrow_move_assignable<value_type>::value
    )
  : impl_{std::move(buf), comp} {
    erase(sort_and_unique(begin(), end(), key_compare()), end());
  }
  // clang-format on

  // clang-format off
  flat_set(std::initializer_list<value_type> il,
           const key_compare& comp = key_compare())
    noexcept(
      noexcept(flat_set(underlying_type{il}, comp))
    )
  : flat_set(underlying_type{il}, comp) {}
  // clang-format on

  // clang-format off
  template <typename I>
  // requires InputIterator<I>
  flat_set(I f, I l, const key_compare& comp = key_compare())
    noexcept(
      noexcept(flat_set(underlying_type(f, l), comp))
    )
  : flat_set(underlying_type(f, l), comp) {}
  // clang-format on

  void clear() noexcept { body().clear(); }

  size_type size() const noexcept { return body().size(); }
  size_type max_size() const noexcept { return body().max_size(); }

  bool empty() const noexcept { return body().empty(); }

  iterator begin() noexcept { return body().begin(); }
  const_iterator begin() const noexcept { return body().begin(); }
  const_iterator cbegin() const noexcept { return body().cbegin(); }

  iterator end() noexcept { return body().end(); }
  const_iterator end() const noexcept { return body().end(); }
  const_iterator cend() const noexcept { return body().cend(); }

  reverse_iterator rbegin() noexcept { return body().rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return body().rbegin(); }
  const_reverse_iterator crbegin() const noexcept { return body().crbegin(); }

  reverse_iterator rend() noexcept { return body().rend(); }
  const_reverse_iterator rend() const noexcept { return body().rend(); }
  const_reverse_iterator crend() const noexcept { return body().crend(); }

  key_compare key_comp() const noexcept { return std::get<1>(impl_); }
  value_compare value_comp() const noexcept { return key_comp(); }

  underlying_type& body() noexcept { return std::get<0>(impl_); }
  const underlying_type& body() const noexcept { return std::get<0>(impl_); }

  void erase(const_iterator f, const_iterator l) { body().erase(f, l); }

  friend bool operator==(const flat_set& x, const flat_set& y) {
    return x.body() == y.body();
  }

  friend bool operator!=(const flat_set& x, const flat_set& y) {
    return !(x == y);
  }

  friend bool operator<(const flat_set& x, const flat_set& y) {
    return x.body() < y.body();
  }

  friend bool operator>(const flat_set& x, const flat_set& y) { return y < x; }

  friend bool operator<=(const flat_set& x, const flat_set& y) {
    return !(y < x);
  }

  friend bool operator>=(const flat_set& x, const flat_set& y) {
    return !(x < y);
  }
};

}  // namespace lib
