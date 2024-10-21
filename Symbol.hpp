#pragma once
#include <string_view>
#include <string>

namespace hft {

class Symbol {
  std::array<char, 8> _data;
  std::string_view _view;

 public:

  Symbol() {
    std::fill(_data.begin(), _data.end(), 0);
  }

  auto operator==(const char* str) -> bool {
    return _view == std::string_view(str);
  }
  friend std::istream& operator>>(std::istream& os, Symbol& s);
  friend std::ostream& operator<<(std::ostream& os, const Symbol& s);
};

std::ostream& operator<<(std::ostream& os, const Symbol& s) {
  os << s._view;
  return os;
}

std::istream& operator>>(std::istream& is, Symbol& s) {
  if (std::iswspace(is.peek())) {
    is.ignore();
  }
  int i = 0;
  while (!std::iswspace(is.peek()) && i < 8) {
    s._data[i++] = is.get();
  }
  s._view = std::string_view(s._data.data());
  return is;
}

}  // end namespace hft

// namespace std {
//   template <> struct hash<hft::Symbol>
//   {
//     size_t operator()(const hft::Symbol & x) const
//     {
//       return std::hash<std::string_view>{}(std::string_view(x));
//     }
//   };
// }
