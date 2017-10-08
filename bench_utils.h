#include <utility>

template <typename T, size_t alginment = 1024>
// requires Integral<T>
class alignas(alginment) padded_int {
 public:
  padded_int() {}
  padded_int(T x) : value_(std::move(x)) {}
  padded_int& operator++() {
    value_++;
    return *this;
  }

  padded_int operator%(int x) {
    return value_ % x;
  }

  explicit operator bool() { return bool(value_); }

  friend bool operator<(const padded_int& x, const padded_int& y) {
    return x.value_ < y.value_;
  }

  friend bool operator>(const padded_int& x, const padded_int& y) {
    return y < x;
  }

  friend bool operator<=(const padded_int& x, const padded_int& y) {
    return !(y < x);
  }

  friend bool operator>=(const padded_int& x, const padded_int& y) {
    return !(x < y);
  }

 private:
  T value_ = 0;
};

struct compare_by_first {
  template <typename T1, typename T2, typename U>
  bool operator()(const std::pair<T1, T2>& x, const U& y) {
    return x.first < y;
  }

  template <typename T, typename U1, typename U2>
  bool operator()(const T& x, const std::pair<U1, U2>& y) {
    return x < y.first;
  }

  template <typename T1, typename T2, typename U1, typename U2>
  bool operator()(const std::pair<T1, T2>& x, const std::pair<U1, U2>& y) {
    return x.first < y.first;
  }
};
