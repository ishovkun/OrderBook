#include <iostream>
#include <iomanip>
#include "../Price.hpp"
#include "../SymbolBook.hpp"

using namespace hft;

#define CHECK_EMPTY(container) \
  if (!container.empty()) { \
    std::cout << "Container is not empty" << std::endl; \
    return false; \
  }

#define CHECK_EQUAL(a, b) \
  if (a != b) { \
    std::cout << #a << " (" << a << ") != " << #b << " (" << b << ")" << std::endl; \
    return false; \
  }

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

auto test_symbol_book() -> bool {
    // "O 10000 IBM B 10 100.00000"            | results.size() == 0
    // "O 10001 IBM B 10 99.00000"             | results.size() == 0
    // "O 10002 IBM S 5 101.00000"             | results.size() == 0
    // "O 10003 IBM S 5 100.00000"             | results.size() == 2
  std::cout << "testing the book" << std::endl;
  SymbolBook book;
  std::vector<Result> results;
  book.add(SymbolOrder(10000, Side::Buy, 10, Price("100.00000")), results);
  CHECK_EMPTY(results);
  book.add(SymbolOrder(10001, Side::Buy, 10, Price("99.00000")), results);
  CHECK_EMPTY(results);
  book.add(SymbolOrder(10002, Side::Sell, 5, Price("101.00000")), results);
  CHECK_EMPTY(results);
  book.add(SymbolOrder(10003, Side::Sell, 5, Price("100.00000")), results);
  CHECK_EQUAL(results.size(), 2);
  for (auto const & r : results) {
    CHECK_EQUAL(r.type, ResultType::FillConfirm);
  }
  CHECK_EQUAL(results[0].order_id, 10003);
  CHECK_EQUAL(results[0].quantity, 5);
  CHECK_EQUAL(results[1].order_id, 10000);
  CHECK_EQUAL(results[1].quantity, 5);

  results.clear();
  book.add(SymbolOrder(10002, Side::Sell, 5, Price("101.00000")), results);
  CHECK_EQUAL(results.size(), 1);
  CHECK_EQUAL(results[0].order_id, 10002);
  CHECK_EQUAL(results[0].type, ResultType::Error);


  return true;
}

template <typename F>
void run_test(F f, std::string const & name) {
  if (!f()) {
    std::cout << std::setw(20) << std::left << name << " [fail ❌]" << std::endl;
  }
  else {
    std::cout << std::setw(20) <<std::left << name << " [pass ✅]" << std::endl;
  }
}

auto main(int , char *[]) -> int {

  run_test(test_price, "Price");
  run_test(test_symbol_book, "Symbol book");

  // std::cout << std::numeric_limits<uint64_t>::max() << std::endl;
  // std::cout << std::numeric_limits<int64_t>::max() << std::endl;
  // std::cout << std::numeric_limits<int64_t>::max() / double(1e12) << std::endl;
  // std::cout << std::numeric_limits<int64_t>::max() / double(1e12) << std::endl;


  return 0;
}
