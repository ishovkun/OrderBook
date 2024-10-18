#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

// (7.5 format means up to 7 digits before the decimal
// and exactly 5 digits after the decimal)
class Price {
 private:
  int64_t _val;

  constexpr static int32_t MAX_INTEGRAL = 100'000'000;  // 10^8
  constexpr static int32_t MAX_DECIMAL = 1'000'000;        // 10^6
 public:
  Price() : _val(0) {}

  explicit Price(int64_t val) : _val(val)
  {
    checkOverflow_(val);
  }

  Price(std::string_view s) {
    auto dot = s.find('.');
    if (dot == std::string_view::npos) {
      throw std::invalid_argument("No decimal point in the price");
    }
    if (dot > 7) {
      throw std::invalid_argument("Integral part of the price is too big");
    }
    if (s.size() - dot - 1 != 5) {
      throw std::invalid_argument("Decimal part of the price is too big");
    }

    auto integral = std::stoll(std::string(s.substr(0, dot)));
    auto decimal = std::stoll(std::string(s.substr(dot + 1)));

    if (integral < 0 || decimal < 0) {
      throw std::invalid_argument("Negative price");
    }

    if (decimal >= MAX_DECIMAL) {
      throw std::invalid_argument("Decimal part of the price is too big");
    }

    if (integral >= MAX_INTEGRAL) {
      throw std::invalid_argument("Integral part of the price is too big");
    }

    _val = integral * MAX_DECIMAL + decimal;
  }

  friend auto operator+(Price const & lhs, Price const & rhs) -> Price;

  auto operator==(Price const & other) const -> bool  {
    return _val == other._val;
  }

  auto operator<(Price const & other) const -> bool  {
    return _val < other._val;
  }

  auto operator!=(Price const & other) const -> bool  {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os, const Price& price);

private:
  auto checkOverflow_(int64_t val) const ->void {
    if (val / MAX_DECIMAL >= MAX_INTEGRAL) {
      throw std::invalid_argument("Decimal part of the price is too big");
    }
  }
};

auto operator+(Price const & lhs, Price const & rhs) -> Price {
  auto ans = Price(lhs._val + rhs._val);
  ans.checkOverflow_(ans._val);
  return ans;
}

std::ostream& operator<<(std::ostream& os, const Price& price) {
  os << price._val / Price::MAX_DECIMAL << "." << price._val % Price::MAX_DECIMAL;
  return os;
}
