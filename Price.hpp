#pragma once
#include <cstdint>
#include <stdexcept>

// (7.5 format means up to 7 digits before the decimal
// and exactly 5 digits after the decimal)
class Price {
 private:
  int32_t _integral;
  int32_t _decimal;

  constexpr static int32_t MAX_INTEGRAL = 100'000'000;  // 10^8
  constexpr static int32_t MAX_DECIMAL = 1'000'000;        // 10^6
 public:
  Price() : _integral(0), _decimal(0) {}

  explicit Price(int32_t integral, int32_t decimal)
      : _integral(integral), _decimal(decimal)
  {
    check_overflow(integral, decimal);
  }

  Price & operator+(Price const & other) {
    _integral += other._integral;
    auto dec = _decimal + other._decimal;
    _integral += dec / MAX_DECIMAL;
    _decimal = dec % MAX_DECIMAL;
    check_overflow(_integral, _decimal);
    return *this;
  }

  auto operator==(Price const & other) const -> bool  {
    return _integral == other._integral && _decimal == other._decimal;
  }

  auto operator<(Price const & other) const -> bool  {
    return _integral < other._integral || (_integral == other._integral && _decimal < other._decimal);
  }

  auto operator!=(Price const & other) const -> bool  {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os, const Price& price);
private:
  void check_overflow(int32_t integral, int32_t decimal) {
    if (decimal >= MAX_DECIMAL) {
      throw std::invalid_argument("Decimal part of the price is too big");
    }
    if (decimal < 0) {
      throw std::invalid_argument("Decimal part of the price is negative");
    }
    if (integral >= MAX_INTEGRAL) {
      throw std::invalid_argument("Price higher than the limit");
    }
  }
};

std::ostream& operator<<(std::ostream& os, const Price& price) {
  os << price._integral << "." << price._decimal;
  return os;
}
