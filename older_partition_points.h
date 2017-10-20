#include <cassert>
#include <utility>

namespace v1 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
  auto len = std::distance(f, l);
  auto step = 1;
  while (len > step) {
    I test = std::next(f, step);
    if (!p(*test)) {
      l = test;
      break;
    }
    f = ++test;
    len -= step + 1;
    step <<= 1;
  }
  return std::partition_point(f, l, p);
}

}  // namespace v1

namespace v2 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
  auto len = std::distance(f, l);
  if (len < 5)
    return std::find_if_not(f, l, p);

  auto middle = std::next(f, len / 2);
  if (p(*middle))
    return std::partition_point(f, l, p);

  auto step = 1;
  while (true) {
    I test = std::next(f, step);
    if (!p(*test)) {
      return std::partition_point(f, test, p);
    }
    f = ++test;
    step <<= 1;
  }
}

}  // namespace v2

namespace v3 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
  // clang-format off
  if (f == l || !p(*f)) return f; ++f;
  if (f == l || !p(*f)) return f; ++f;
  if (f == l || !p(*f)) return f; ++f;
  if (f == l || !p(*f)) return f; ++f;
  // clang-format on

  auto len = std::distance(f, l);
  if (len < 5)
    return partition_point_biased(f, l, p);

  auto middle = std::next(f, len / 2);
  if (p(*middle))
    return std::partition_point(f, l, p);

  auto step = 1;
  while (true) {
    I test = std::next(f, step);
    if (!p(*test)) {
      return std::partition_point(f, test, p);
    }
    f = ++test;
    step <<= 1;
  }
}

}  // namespace v3

namespace v4 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
  auto len = std::distance(f, l);
  auto middle = std::next(f, len / 2);
  if (middle == l || p(*middle))
    return std::partition_point(f, l, p);

  // clang-format off
  if (!p(*f)) return f; ++f;
  if (!p(*f)) return f; ++f;
  if (!p(*f)) return f; ++f;
  if (!p(*f)) return f; ++f;
  // clang-format on

  auto step = 1;
  while (true) {
    I test = std::next(f, step);
    if (!p(*test)) {
      return std::partition_point(f, test, p);
    }
    f = ++test;
    step <<= 1;
  }
}

}  // namespace v4

namespace copied {

template<typename I, typename P>
I partition_point(I f, I l, P p) {
  auto len = std::distance(f, l);
  while (len) {
    auto len2 = len / 2;
    I middle = std::next(f, len2);
    if (p(*middle)) {
      f = ++middle;
      len -= len2 + 1;
    } else {
      len = len2;
    }
  }
  return f;
}

}  // namespace copied

namespace lib {

namespace detail {

template <typename I, typename O>
O copy(I f, I l, O o);

}  // namespace detail

}  // namespace lib

namespace v5 {

template <typename I, typename P>
std::pair<I, I> partition_point_biased_going_in_powers(I f,
                                                       I middle,
                                                       I l,
                                                       P p) {
    auto step = 1;
    while (true) {
      I test = std::next(f, step);
      if (!p(*test)) {
        return {std::partition_point(f, test, p), middle};
      }
      f = ++test;
      step <<= 1;
    }
}

template <typename I, typename P>
std::pair<I, I> partition_point_uncontrollable_forward_progress(I f,
                                                                I middle,
                                                                I l,
                                                                P p) {
  while (true) {
    // clang-format off
    if (!p(*f)) return {f, middle}; ++f;
    if (!p(*f)) return {f, middle}; ++f;
    if (!p(*f)) return {f, middle}; ++f;
    if (!p(*f)) return {f, middle}; ++f;
    // clang-format on

    auto step = 1;
    while (true) {
      I test = std::next(f, step);
      if (!p(*test)) {
        break;
      }
      f = ++test;
      step <<= 1;
    }
  }
}

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
std::pair<I, I> partition_point_biased_repeated(I f, I middle, I l, P p) {
  while (true) {
    if (!p(*middle)) {
      return partition_point_uncontrollable_forward_progress(f, middle, l, p);
    }
    f = std::max(f, ++middle);
    if (f == l)
      return {l, l};
    middle = std::next(f, std::distance(f, l) / 2);
  }
}

template <typename I, typename V, typename P>
inline
// requires ForwardIterator<I> && StrictWeakOrdering<P, ValueType<I>>
std::pair<I, I> lower_bound_biased_repeated(I f, I m, I l, const V& v, P p) {
  auto less_than_v =
      [&](typename std::iterator_traits<I>::reference x) { return p(x, v); };
  return partition_point_biased_repeated(f, m, l, less_than_v);
  //return partition_point_biased(f, l, less_than_v);
}

template <typename I, typename V, typename O, typename P>
inline
std::tuple<I, I, O> copy_unitl_lower_bound_repeated(I f,
                                                    I m,
                                                    I l,
                                                    const V& v,
                                                    O o,
                                                    P p) {
  I new_f;
  std::tie(new_f, m) = lower_bound_biased_repeated(f, m, l, v, p);
  return {new_f, m, lib::detail::copy(f, new_f, o)};
}

template <typename I1, typename I2, typename O, typename P>
// requires ForwardIterator<I1> && ForwardIterator<I2> && OutputIterator<O> &&
//          StrictWeakOrdering<P(ValueType<I>)>
std::tuple<I1, I2, O> set_union_intersecting_parts(I1 f1,
                                                   I1 l1,
                                                   I2 f2,
                                                   I2 l2,
                                                   O o,
                                                   P p) {
  I1 m1 = f1;
  I2 m2 = f2;

  while (f1 != l1 && f2 != l2) {
    std::tie(f1, m1, o) =
        copy_unitl_lower_bound_repeated(f1, m1, l1, *f2, o, p);

    if (f1 == l1)
      break;

    std::tie(f2, m2, o) =
        copy_unitl_lower_bound_repeated(f2, m2, l2, *f1, o, p);

    if (f2 == l2)
      break;

    if (!p(*f1, *f2))
      ++f2;
  }

  return {f1, f2, o};
}

}  // namespace v5



