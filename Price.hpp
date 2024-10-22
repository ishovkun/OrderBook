#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <iomanip>

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

  static auto fromString(std::string_view s, Price &p) -> bool;

  explicit Price(std::string_view s) {
    if (!Price::fromString(s, *this)) {
      throw std::invalid_argument("Invalid price format");
    }
  }

  auto operator==(Price const & other) const -> bool  {
    return _val == other._val;
  }

  auto operator<(Price const & other) const -> bool  {
    return _val < other._val;
  }

  auto operator>=(Price const & other) const -> bool  {
    return _val >= other._val;
  }

  auto operator<=(Price const & other) const -> bool  {
    return _val <= other._val;
  }

  auto operator!=(Price const & other) const -> bool  {
    return !(*this == other);
  }

  friend auto operator+(Price const & lhs, Price const & rhs) -> Price;
  friend auto operator-(Price const & lhs, Price const & rhs) -> Price;
  friend std::ostream& operator<<(std::ostream& os, const Price& price);
  friend std::istream& operator>>(std::istream& os, Price& price);

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

auto operator-(Price const & lhs, Price const & rhs) -> Price {
  auto ans = Price(lhs._val - rhs._val);
  ans.checkOverflow_(ans._val);
  return ans;
}

std::ostream& operator<<(std::ostream& os, const Price& price) {
  os << price._val / Price::MAX_DECIMAL << "." << std::setw(5) << std::setfill('0') << (price._val % Price::MAX_DECIMAL);
  return os;
}
std::istream& operator>>(std::istream& os, Price& price) {
  std::string s;
  os >> s;
  Price::fromString(s, price);
  return os;
}

bool Price::fromString(std::string_view s, Price & p) {
  auto dot = s.find('.');
  if (dot == std::string_view::npos) {
    return false;
  }
  if (dot > 7) {
    return false;
  }
  if (s.size() - dot - 1 != 5) {
    return false;
  }

  auto integral = std::stoll(std::string(s.substr(0, dot)));
  auto decimal = std::stoll(std::string(s.substr(dot + 1)));

  if (integral < 0 || decimal < 0) {
    return false;
  }

  if (decimal >= Price::MAX_DECIMAL) {
    return false;
  }

  if (integral >= Price::MAX_INTEGRAL) {
    return false;
  }

  p._val = integral * Price::MAX_DECIMAL + decimal;
  return true;
}
