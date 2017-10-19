#include <cassert>

namespace v1 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p) {
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
  if (p(*middle))
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


