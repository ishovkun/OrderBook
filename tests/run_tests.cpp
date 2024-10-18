#include <iostream>
#include "../Price.hpp"

auto test_price() -> bool {
  Price p1("100.00000");
  Price p2("100.00000");
  auto p3 = p1 + p2;
  if (p3 != Price("200.00000")) {
    std::cout << p3 << " != "<< Price("200.00000") << std::endl;
    return false;
  }
  // test overflow
  try {
    auto p4 = p1 + Price("10'000'000.00");
    std::cout << "Should have thrown p4 = " << p4 << std::endl;
    auto p5 = p1 + Price("0.1000001");
    std::cout << "Should have thrown p5 = " << p5 << std::endl;
    return false;
  } catch (std::invalid_argument const & e) {
    // expected
  }
  return true;
}

template <typename F>
void run_test(F f, std::string const & name) {
  if (!f()) {
    std::cout << name << " [failed X]" << std::endl;
  }
  else {
    std::cout << name << " [pass]" << std::endl;
  }
}

auto main(int , char *[]) -> int {

  run_test(test_price, "Price");

  std::cout << std::numeric_limits<uint64_t>::max() << std::endl;
  std::cout << std::numeric_limits<int64_t>::max() << std::endl;
  std::cout << std::numeric_limits<int64_t>::max() / double(1e12) << std::endl;
  std::cout << std::numeric_limits<int64_t>::max() / double(1e12) << std::endl;


  return 0;
}
