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
