#include <iostream>
#include "../Price.hpp"

auto test_price() -> bool {
  Price p1(100, 00000);
  Price p2(100, 0);
  auto p3 = p1 + p2;
  if (p3 != Price(200, 00000)) {
    std::cout << p3 << " != "<< Price(200, 00000) << std::endl;
    return false;
  }
  // test overflow
  try {
    auto p4 = p1 + Price((int32_t)200'000'000, 1);
    std::cout << "Should have thrown p4 = " << p4 << std::endl;
    auto p5 = p1 + Price((int32_t)0, 1'000'001);
    std::cout << "Should have thrown p5 = " << p5 << std::endl;
    return false;
  } catch (std::invalid_argument const & e) {
    // expected
  }
  // test sum overflow
  auto p4 = Price(1'00'000'000-50, 0);
  try {
    auto p5 = p3 + p4;
    std::cout << "Should have thrown p5 = " << p5 << std::endl;
    return false;
  }
  catch (std::invalid_argument const & e) {
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


  return 0;
}
