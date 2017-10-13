#include <utility>

template <typename T, size_t alginment = 2 * sizeof(T)>
// requires Integral<T>
class alignas(alginment) padded_int {
 public:
  padded_int() = default;
  padded_int(T x) : value_(std::move(x)) {}
  padded_int& operator++() {
    value_++;
    return *this;
  }

  explicit operator bool() { return bool(value_); }

  padded_int& operator%=(const padded_int& x) {
    value_ %= x.value_;
    return *this;
  }

  friend padded_int operator%(padded_int x, const padded_int& y) {
    x %= y;
    return x;
  }

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

template <typename T>
class non_trivial {
 public:
  non_trivial() noexcept;
  non_trivial(T value_) noexcept : value_(std::move(value_)) {}
  non_trivial(const non_trivial&) noexcept;
  non_trivial(non_trivial&&) noexcept;
  non_trivial& operator=(const non_trivial&) noexcept;
  non_trivial& operator=(non_trivial&&) noexcept;
  ~non_trivial();

  explicit operator bool() { return bool(value_); }

  non_trivial& operator%=(const non_trivial& x) {
    value_ %= x.value_;
    return *this;
  }

  friend non_trivial operator%(non_trivial x, const non_trivial& y) {
    x %= y;
    return x;
  }

  friend bool operator<(const non_trivial& x, const non_trivial& y) {
    return x.value_ < y.value_;
  }

  friend bool operator>(const non_trivial& x, const non_trivial& y) {
    return y < x;
  }

  friend bool operator<=(const non_trivial& x, const non_trivial& y) {
    return !(y < x);
  }

  friend bool operator>=(const non_trivial& x, const non_trivial& y) {
    return !(x < y);
  }
private:
  T value_ = 0;
};

template <typename T>
non_trivial<T>::non_trivial() noexcept = default;

template<typename T>
non_trivial<T>::non_trivial(const non_trivial&) noexcept = default;

template <typename T>
non_trivial<T>::non_trivial(non_trivial&&) noexcept = default;

template <typename T>
auto non_trivial<T>::operator=(const non_trivial&) noexcept ->
    non_trivial& = default;

template <typename T>
auto non_trivial<T>::operator=(non_trivial&&) noexcept ->
   non_trivial& = default;

template <typename T>
non_trivial<T>::~non_trivial() = default;

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
