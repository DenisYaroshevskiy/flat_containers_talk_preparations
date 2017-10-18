#pragma once

#include <functional>
#include <tuple>
#include <vector>

namespace better_linear {

template <typename C>
using Iterator = typename C::iterator;

template <typename I>
using Reference = typename std::iterator_traits<I>::reference;

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

template <typename F>
// requires Predicate<F>
struct not_fn_t {
  F f;

  template <typename... Args>
  bool operator()(Args&&... args) {
    return !f(std::forward<Args>(args)...);
  }
};

template <typename F>
not_fn_t<F> not_fn(F f) noexcept {
  return {f};
}

template <typename F>
struct inverse_t {
  F f;

  template <typename X, typename Y>
  bool operator()(const X& x, const Y& y) {
    return f(y, x);
  }
};

template <typename F>
inverse_t<F> inverse_fn(F f) noexcept {
  return {f};
}



template <typename I, typename Comparator>
I sort_and_unique(I f, I l, Comparator comp) {
  std::sort(f, l, comp);
  return std::unique(f, l, not_fn(comp));
}



template <class InputIt1, class InputIt2, class OutputIt, class Compare>
std::tuple<InputIt1, InputIt2, OutputIt> std_set_union_intersecting_parts(
    InputIt1 first1,
    InputIt1 last1,
    InputIt2 first2,
    InputIt2 last2,
    OutputIt d_first,
    Compare comp) {
  for (; first1 != last1; ++d_first) {
    if (first2 == last2)
      break;
    if (comp(*first2, *first1)) {
      *d_first = *first2++;
    } else {
      *d_first = *first1;
      if (!comp(*first1, *first2))
        ++first2;
      ++first1;
    }
  }
  return {first1, first2, d_first};
}

template <typename I1, typename I2, typename P>
// requires ForwardIterator<I1> && ForwardIterator<I2> &&
//          StrictWeakOrdering<P, ValueType<I>>
std::pair<I1, I1> set_union_into_tail(I1 buf, I1 f1, I1 l1, I2 f2, I2 l2, P p) {
  std::move_iterator<I1> move_f1;
  std::tie(move_f1, f2, buf) =
      std_set_union_intersecting_parts(std::make_move_iterator(f1),  //
                                       std::make_move_iterator(l1),  //
                                       f2, l2,                       //
                                       buf, p);                      //

  return {std::copy(f2, l2, buf), move_f1.base()};
}

template <typename C, typename I, typename P>
// requires  Container<C> &&  ForwardIterator<I> &&
//           StrictWeakOrdering<P(ValueType<C>)>
void insert_first_last_impl(C& c, I f, I l, P p) {
  auto new_len = std::distance(f, l);
  auto orig_len = c.size();
  c.resize(orig_len + 2 * new_len);

  auto orig_f = c.begin();
  auto orig_l = c.begin() + orig_len;
  auto f_in = c.end() - new_len;
  auto l_in = c.end();
  auto buf = f_in;

  std::copy(f, l, f_in);
  l_in = sort_and_unique(f_in, l_in, p);

  using reverse_it = typename C::reverse_iterator;
  auto move_reverse_it =
      [](Iterator<C> it) { return std::make_move_iterator(reverse_it(it)); };

  auto reverse_remainig_buf_range = better_linear::set_union_into_tail(
      reverse_it(buf), reverse_it(orig_l), reverse_it(orig_f),
      move_reverse_it(l_in), move_reverse_it(f_in), inverse_fn(p));

  auto remaining_buf =
      std::make_pair(reverse_remainig_buf_range.second.base() - c.begin(),
                     reverse_remainig_buf_range.first.base() - c.begin());

  c.erase(c.end() - new_len, c.end());
  c.erase(c.begin() + remaining_buf.first, c.begin() + remaining_buf.second);
}

template <typename Key>
class flat_set {
  std::vector<Key> body_;

 public:
  template <typename I>
  flat_set(I f, I l)
      : body_(f, l) {}

  template <typename I>
  void insert(I f, I l) {
    insert_first_last_impl(body_, f, l, std::less<>{});
  }
};

}  // namespace  better_linear
