#include <algorithm>

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

startWithFirst:
  while (__builtin_expect(comp(*f1, *f2), true)) {
    *o++ = *f1++;
    if (f1 == l1) goto copySecond;
  }

  if (comp(*f2, *f1)) *o++ = *f2;
  ++f2; if (f2 == l2) goto copyFirst;

  while (__builtin_expect(comp(*f2, *f1), true)) {
    *o++ = *f2++;
    if (f2 == l2) goto copyFirst;
  }

  if (comp(*f1, *f2)) {
    *o++ = *f1++;
    if (f1 == l1) goto copySecond;
    goto startWithFirst;
  }

  ++f2; if (f2 == l2) goto copyFirst;
  goto startWithFirst;

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

  while (__builtin_expect(comp(*f1, *f2), true)) {
  assignFirst:
    *o++ = *f1++;
    if (f1 == l1) goto copySecond;
  }

  if (__builtin_expect(comp(*f2, *f1), true)) goto assignSecond;
  ++f2;
  goto checkSecond;

  while (__builtin_expect(comp(*f2, *f1), true)) {
  assignSecond:
    *o++ = *f2++;
  checkSecond:
    if (f2 == l2) goto copyFirst;
  }

  if (comp(*f1, *f2)) goto assignFirst;
  ++f2;
  goto checkSecond;

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

  while (__builtin_expect(comp(*f1, *f2), true)) {
  assignFirst:
    *o++ = *f1++;
    if (f1 == l1) goto copySecond;
  }

  while (__builtin_expect(comp(*f2, *f1), true)) {
    *o++ = *f2++;
  checkSecond:
    if (f2 == l2) goto copyFirst;
  }

  if (comp(*f1, *f2)) goto assignFirst;
  ++f2; goto checkSecond;

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v8
