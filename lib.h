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

  template <typename... Args>
  bool operator()(Args&&... args) {
    return !f(std::forward<Args>(args)...);
  }
};

// clang-format off
template <typename ContainerValueType, typename InsertedType>
using insert_should_be_enabled =
typename std::enable_if<
  std::is_same<
    ContainerValueType,
    typename std::remove_cv<
      typename std::remove_reference<InsertedType>::type
    >::type
  >::value
>::type;
// clang-format on

}  // namespace detail

// type functions ------------------------------------------------------------

template <typename I>
using Reference = typename std::iterator_traits<I>::reference;

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
  auto less_than_v = [&](Reference<I> x) { return p(x, v); };
  return partition_point_biased(f, l, less_than_v);
}

template <typename I, typename V>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_biased(I f, I l, const V& v) {
  return lower_bound_biased(f, l, v, less{});
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I upper_bound_biased(I f, I l, const V& v, P p) {
  auto greater_than_v = [&](Reference<I> x) { return !p(v, x); };
  return partition_point_biased(f, l, greater_than_v);
}

template <typename I, typename V>
// requires ForwardIterator<I> && TotallyOrdered<ValueType<I>>
I upper_bound_biased(I f, I l, const V& v) {
  return upper_bound_biased(f, l, v, less{});
}

template <typename I, typename P>
// requires BidirectionalIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_hinted(I f, I hint, I l, P p) {
  I rhs = partition_point_biased(hint, l, p);
  if (rhs != hint)
    return rhs;

  return partition_point_biased(std::reverse_iterator<I>(hint),
                                std::reverse_iterator<I>(f), not_fn(p)).base();
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I lower_bound_hinted(I f, I hint, I l, const V& v, P p) {
  auto less_than_v = [&](Reference<I> x) { return p(x, v); };
  return partition_point_hinted(f, hint, l, less_than_v);
}

template <typename I, typename V>
// requires ForwardIterator<I> && TotallyOrdered<ValueType<I>>
I lower_bound_hinted(I f, I hint, I l, const V& v) {
  return lower_bound_hinted(f, hint, l, v, less{});
}

template <typename I, typename V, typename P>
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
I upper_bound_hinted(I f, I hint, I l, const V& v, P p) {
  auto greater_than_v = [&](Reference<I> x) { return !p(v, x); };
  return partition_point_hinted(f, hint, l, greater_than_v);
}

template <typename I, typename V>
// requires ForwardIterator<I> && TotallyOrdered<ValueType<I>>
I upper_bound_hinted(I f, I hint, I l, const V& v) {
  return upper_bound_hinted(f, hint, l, v, less{});
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

  template <typename V>
  using type_for_value_compare =
      typename std::conditional<TransparentComparator<value_compare>(),
                                V,
                                value_type>::type;

  iterator const_cast_iterator(const_iterator c_it) {
    return begin() + std::distance(cbegin(), c_it);
  }

 public:
  // --------------------------------------------------------------------------
  // Lifetime -----------------------------------------------------------------

  flat_set() = default;
  explicit flat_set(const key_compare& comp) : impl_{{}, comp} {}

  template <typename I>
  // requires InputIterator<I>
  flat_set(I f, I l, const key_compare& comp = key_compare())
      : flat_set(underlying_type(f, l), comp) {}

  flat_set(const flat_set&) = default;
  flat_set(flat_set&&) = default;

  flat_set(underlying_type buf, const key_compare& comp = key_compare())
      : impl_{std::move(buf), comp} {
    erase(sort_and_unique(begin(), end(), key_compare()), end());
  }

  flat_set(std::initializer_list<value_type> il,
           const key_compare& comp = key_compare())
      : flat_set(underlying_type{il}, comp) {}

  ~flat_set() = default;

  // --------------------------------------------------------------------------
  // Assignments --------------------------------------------------------------

  flat_set& operator=(const flat_set&) = default;
  flat_set& operator=(flat_set&&) = default;
  flat_set& operator=(std::initializer_list<value_type> il) {
    *this = flat_set{il};
    return *this;
  }

  //---------------------------------------------------------------------------
  // Memory management.

  void reserve(size_type new_capacity) { body().reserve(new_capacity); }
  size_type capacity() const { return body().capacity(); }
  void shrink_to_fit() { body().shrink_to_fit(); }

  //---------------------------------------------------------------------------
  // Size management.

  void clear() { body().clear(); }

  size_type size() const { return body().size(); }
  size_type max_size() const { return body().max_size(); }

  bool empty() const { return body().empty(); }

  //---------------------------------------------------------------------------
  // Iterators.

  iterator begin() { return body().begin(); }
  const_iterator begin() const { return body().begin(); }
  const_iterator cbegin() const { return body().cbegin(); }

  iterator end() { return body().end(); }
  const_iterator end() const { return body().end(); }
  const_iterator cend() const { return body().cend(); }

  reverse_iterator rbegin() { return body().rbegin(); }
  const_reverse_iterator rbegin() const { return body().rbegin(); }
  const_reverse_iterator crbegin() const { return body().crbegin(); }

  reverse_iterator rend() { return body().rend(); }
  const_reverse_iterator rend() const { return body().rend(); }
  const_reverse_iterator crend() const { return body().crend(); }

  //---------------------------------------------------------------------------
  // Insert operations.

  template <typename V,
            typename = detail::insert_should_be_enabled<value_type, V>>
  std::pair<iterator, bool> insert(V&& v) {
    iterator pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {body().insert(pos, std::forward<V>(v)), true};
    return {pos, false};
  }

  template <typename V,
            typename = detail::insert_should_be_enabled<value_type, V>>
  iterator insert(const_iterator hint, V&& v) {
    auto pos = lower_bound_hinted(cbegin(), hint, cend(), v, value_comp());
    if (pos == end() || value_comp()(v, *pos))
      return body().insert(pos, std::forward<V>(v));
    return const_cast_iterator(pos);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(value_type{std::forward<Args>(args)...});
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args) {
    return insert(hint, value_type{std::forward<Args>(args)...});
  }

  // --------------------------------------------------------------------------
  // Erase operations.

  iterator erase(iterator pos) { return body().erase(pos); }
  iterator erase(const_iterator pos) { return body().erase(pos); }

  iterator erase(const_iterator f, const_iterator l) {
    return body().erase(f, l);
  }

  template <typename V>
  size_type erase(const V& v) {
    auto eq_range = equal_range(v);
    size_type res = std::distance(eq_range.first, eq_range.second);
    erase(eq_range.first, eq_range.second);
    return res;
  }

  // --------------------------------------------------------------------------
  // Search operations.

  template <typename V>
  size_type count(const V& v) const {
    auto eq_range = equal_range(v);
    return std::distance(eq_range.first, eq_range.second);
  }

  template <typename V>
  iterator find(const V& v) {
    auto eq_range = equal_range(v);
    return (eq_range.first == eq_range.second) ? end() : eq_range.first;
  }

  template <typename V>
  const_iterator find(const V& v) const {
    auto eq_range = equal_range(v);
    return (eq_range.first == eq_range.second) ? end() : eq_range.first;
  }

  template <typename V>
  std::pair<iterator, iterator> equal_range(const V& v) {
    auto pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {pos, pos};

    return {pos, std::next(pos)};
  }

  template <typename V>
  std::pair<const_iterator, const_iterator> equal_range(const V& v) const {
    auto pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {pos, pos};

    return {pos, std::next(pos)};
  }

  template <typename V>
  iterator lower_bound(const V& v) {
    const type_for_value_compare<V>& v_ref = v;
    return std::lower_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  const_iterator lower_bound(const V& v) const {
    const type_for_value_compare<V>& v_ref = v;
    return std::lower_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  iterator upper_bound(const V& v) {
    const type_for_value_compare<V>& v_ref = v;
    return std::upper_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  const_iterator upper_bound(const V& v) const {
    const type_for_value_compare<V>& v_ref = v;
    return std::upper_bound(begin(), end(), v_ref, value_comp());
  }

  //---------------------------------------------------------------------------
  // Getters.

  key_compare key_comp() const { return std::get<1>(impl_); }
  value_compare value_comp() const { return key_comp(); }

  underlying_type& body() { return std::get<0>(impl_); }
  const underlying_type& body() const { return std::get<0>(impl_); }

  //---------------------------------------------------------------------------
  // General operations.

  void swap(flat_set& x) { body().swap(x.body()); }

  friend void swap(flat_set& x, flat_set& y) { x.swap(y); }

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

template <typename Key,
          typename Comparator,
          typename UnderlyingType,
          typename P>
// requires UnaryPredicate<P(reference)>
void erase_if(flat_set<Key, Comparator, UnderlyingType>& x, P p) {
  x.erase(std::remove_if(x.begin(), x.end(), p), x.end());
}

}  // namespace lib
