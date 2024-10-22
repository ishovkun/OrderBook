#include <iostream>
#include <iomanip>
#include "Price.hpp"
#include "OrderMatcher.hpp"
#include "Action.hpp"
#include "MultiSymbolBook.hpp"

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

auto test_order_book() -> bool {
    // "O 10000 IBM B 10 100.00000"            | results.size() == 0
    // "O 10001 IBM B 10 99.00000"             | results.size() == 0
    // "O 10002 IBM S 5 101.00000"             | results.size() == 0
    // "O 10003 IBM S 5 100.00000"             | results.size() == 2
    std::unordered_map<OrderID, Order> orders;
    orders[10000] = Order(10000, "IBM", Side::Buy, 10, Price("100.00000"));
    orders[10001] = Order(10001, "IBM", Side::Buy, 10, Price("99.00000"));
    orders[10002] = Order(10002, "IBM", Side::Sell, 5, Price("101.00000"));
    orders[10003] = Order(10003, "IBM", Side::Sell, 5, Price("100.00000"));
    hft::OrderMatcher matcher(orders, "IBM");
    std::vector<Result> results;

    matcher.add(10000, results);
    CHECK_EMPTY(results);
    matcher.add(10001, results);
    CHECK_EMPTY(results);
    matcher.add(10002, results);
    CHECK_EMPTY(results);
    matcher.add(10003, results);
    CHECK_EQUAL(results.size(), 2);
    for (auto const &r : results) {
      CHECK_EQUAL(r.type, ResultType::FillConfirm);
    }
    CHECK_EQUAL(results[0].order_id, 10003);
    CHECK_EQUAL(results[0].quantity, 5);
    CHECK_EQUAL(results[1].order_id, 10000);
    CHECK_EQUAL(results[1].quantity, 5);

    results.clear();
    return true;
}

auto test_cancellation() -> bool {
  std::unordered_map<OrderID, Order> orders;
  orders[10000] = Order(10000, "Google", Side::Buy, 10, Price("100.00000"));
  orders[10001] = Order(10001, "Google", Side::Buy, 10, Price("99.00000"));
  orders[10002] = Order(10002, "Google", Side::Sell, 5, Price("101.00000"));
  orders[10003] = Order(10003, "Google", Side::Sell, 5, Price("100.00000"));
  hft::OrderMatcher matcher(orders, "Google");
  std::vector<Result> results;

  matcher.add(10000, results);
  matcher.add(10001, results);
  matcher.cancel(10000, results);
  CHECK_EQUAL(results.size(), 1);
  CHECK_EQUAL(results[0].type, ResultType::CancelConfirm);

  results.clear();
  matcher.cancel(666, results);
  CHECK_EQUAL(results[0].type, ResultType::Error);
  return true;
}

auto test_action() -> bool {
  {
    try {
      std::string action_str = "O 10000 IBM B 10 100.00000 ";
      Action action(action_str);
      CHECK_EQUAL(action.type, ActionType::Place);
      CHECK_EQUAL(action.order.id, 10000);
      CHECK_EQUAL(action.order.quantity, 10);
      CHECK_EQUAL(action.order.symbol, "IBM");
    }
    catch (std::invalid_argument const & e) {
      std::cout << "Should not have thrown: " << e.what() << std::endl;
      return false;
    }
  }
  {
    try {
      std::string action_str = "O 10000 IBM B 10 100.00000 beer";
      Action action(action_str);
      return false;
    }
    catch (std::invalid_argument const & e) {
      // expected
    }
  }
  {
    std::string action_str = "X 10002";
    Action action(action_str);
    CHECK_EQUAL(action.type, ActionType::Cancel);

    try {
      std::string action_str = "X 10002 2198";
      Action action(action_str);
    } catch (std::invalid_argument const & e) {
      // expected
    }
  }

  {
    std::string action_str = "P";
    Action action(action_str);
    CHECK_EQUAL(action.type, ActionType::Print);

    try {
      std::string action_str = "125 P 13";
      Action action(action_str);
    } catch (std::invalid_argument const & e) {
      // expected
    }
  }
  return true;
}

auto test_multi_symbol_book() -> bool {
  MultiSymbolBook book;
  book.add(Order(10000, "Apple", Side::Buy, 10, Price("100.00000")));
  CHECK_EMPTY(book.getResults());
  book.add(Order(10000, "Google", Side::Buy, 10, Price("100.00000")));
  CHECK_EQUAL(book.getResults()[0].type, ResultType::Error);
  book.add(Order(10001, "Apple", Side::Buy, 10, Price("100.00000")));
  book.add(Order(10003, "Apple", Side::Buy, 10, Price("110.00000")));
  book.add(Order(10004, "Gogle", Side::Buy, 10, Price("50.00000")));


  int nitems = 10;
  std::vector<Price> prices(nitems);
  int order_id = 1;
  for (auto i = 0; i < nitems; ++i) {
    prices[i] = Price(std::to_string(10+i) + ".00000");
    // std::cout << "P " << order_id << " B " << prices[i] << std::endl;
    book.add(Order(order_id++, "IBM", Side::Buy, 10, prices[i]));
  }
  for (auto i = 0; i < nitems; ++i) {
    auto sell_price = prices[i] - Price("1.00000");
    // std::cout << "P " << order_id << " S " << sell_price << std::endl;
    book.add(Order(order_id++, "IBM", Side::Sell, 10, sell_price));
  }
  book.cancel(10000);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10001);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10003);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10004);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);

  book.print();
  CHECK_EMPTY(book.getResults());

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
  run_test(test_order_book, "Symbol book");
  run_test(test_cancellation, "Symbol book cancel");
  run_test(test_action, "Action");
  run_test(test_multi_symbol_book, "Multi symbol book");

  return 0;
}
