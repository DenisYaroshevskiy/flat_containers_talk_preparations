#include <algorithm>
#include <cassert>
#include <functional>
#include <type_traits>
#include <iterator>

// Workaround https://bugs.llvm.org/show_bug.cgi?id=35202
template <typename P>
struct is_total_ordering : std::false_type {};

template <>
struct is_total_ordering<std::less<>> : std::true_type {};

template <typename T>
struct equality_is_well_defined
    : std::is_integral<typename std::decay<T>::type> {};

template <typename T, typename U, typename P>
using can_use_eqality = std::integral_constant<bool,
  std::is_same<typename std::decay<T>::type,
               typename std::decay<U>::type>::value &&
  equality_is_well_defined<U>::value &&
  is_total_ordering<P>::value>;

template <typename T, typename U, typename P>
typename std::enable_if<!can_use_eqality<T, U, P>::value, bool>::type
not_equal_or_inverse(const T& x, const U& y, P p) { return p(y, x); }

template <typename T, typename U, typename P>
typename std::enable_if<can_use_eqality<T, U, P>::value, bool>::type
not_equal_or_inverse(const T& x, const U& y, P p) { return !(x == y); }

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

namespace v1 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  for (; f1 != l1; ++o) {
    if (f2 == l2)
      return std::copy(f1, l1, o);
    if (comp(*f2, *f1)) {
      *o = *f2;
      ++f2;
    } else {
      *o = *f1;
      if (!comp(*f1, *f2)) // libc++ bug.
        ++f2;
      ++f1;
    }
  }
  return std::copy(f2, l2, o);
}

}  // namespace v1

namespace v2 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  for (; f1 != l1; ++o) {
    if (f2 == l2)
      return std::copy(f1, l1, o);
    if (comp(*f2, *f1)) {
      *o = *f2;
      ++f2;
    } else {
      if (!comp(*f1, *f2))
        ++f2;
      *o = *f1;
      ++f1;
    }
  }
  return std::copy(f2, l2, o);
}

}  // namespace v2

namespace v3 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (comp(*f2, *f1)) {
      *o++ = *f2++;
      if (f2 == l2) goto copyFirst;
    } else {
      if (!comp(*f1, *f2)) {
        ++f2;
        if (f2 == l2) goto copyFirst;
      }
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v3

namespace v4 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (comp(*f1, *f2)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v4

namespace v5 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v5

namespace v6 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v6

namespace v7 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(!comp(*f1, *f2), false)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

  checkSecond:
    if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v7

namespace v8 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(!comp(*f1, *f2), false)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

  checkSecond:
    if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;

    auto next_f1 = std::lower_bound(f1, l1, *f2, comp);
    o = std::copy(f1, next_f1, o);
    f1 = next_f1; if (f1 == l1) goto copySecond;
    goto checkSecond;
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v8

namespace v9 {

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

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(!comp(*f1, *f2), false)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

  checkSecond:
    if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;

    auto next_f1 = partition_point_biased(
        f1, l1, [&](const auto& x) { return comp(x, *f2); });
    o = std::copy(f1, next_f1, o);
    f1 = next_f1; if (f1 == l1) goto copySecond;
    goto checkSecond;
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v9

namespace v10 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased_no_checks(I f, I l, P p) {
  for (int step = 1;; step <<= 1) {
    auto test = std::next(f, step);
    if (!p(*test)) break;
    f = ++test;
  }
  return f;
}

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(!comp(*f1, *f2), false)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

  checkSecond:
    if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;

    auto len = std::distance(f1, l1);
    if (len <= 4) continue;

    auto segment_end = std::next(f1, len / 2);

    if (!comp(*segment_end, *f2))
      segment_end = partition_point_biased_no_checks(
          f1, segment_end, [&](const auto& x) { return comp(x, *f2); });

    o = std::copy(f1, segment_end, o);
    f1 = segment_end;
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v10

namespace v11 {

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_estimation(I f, I l, P p) {
  I sent = std::next(f, static_cast<size_t>(std::distance(f, l)) / 2);
  if (p(*sent)) return sent;

  if (!p(*f)) return f;
  ++f;
  if (!p(*f)) return f;
  ++f;
  if (!p(*f)) return f;
  ++f;
  for (int step = 2;; step <<= 1) {
    auto test = std::next(f, step);
    if (!p(*test)) break;
    f = ++test;
  }
  return f;
}

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(!comp(*f1, *f2), false)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

  checkSecond:
    if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;

    auto segment_end = partition_point_estimation(
        f1, l1, [&](const auto& x) { return comp(x, *f2); });

    o = std::copy(f1, segment_end, o);
    f1 = segment_end; if (f1 == l1) goto copySecond;
  }

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v11

