#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Price.hpp"
#include "OrderMatcher.hpp"
#include "Action.hpp"
#include "MultiSymbolBook.hpp"

using namespace hft;

#define CHECK_EMPTY(container) \
  if (!container.empty()) { \
    std::cout << "Container is not empty at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

#define CHECK_EQUAL(a, b) \
  if (a != b) { \
    std::cout << #a << " (" << a << ") != " << #b << " (" << b << ") at" << __FILE__ << ":" << __LINE__ << std::endl; \
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

auto test_highest_bidder() -> bool {
/*
** ID Side Quantity Price
** i0 B    100      100.00000
** i1 B    100      120.00000
** i2 B    10       59.00000
** i3 B    1500     200.00000
** i4 B    100      100.00000
** i5 S    10       90.00000
** ---->
** F i3 10 90
** F i5 10 90
 */
  {
    std::vector<Order> olist{
        Order(0, "IBM", Side::Buy, 100, Price("100.00000")),
        Order(1, "IBM", Side::Buy, 100, Price("120.00000")),
        Order(2, "IBM", Side::Buy, 10, Price("59.00000")),
        Order(3, "IBM", Side::Buy, 1500, Price("200.00000")),
        Order(4, "IBM", Side::Buy, 100, Price("100.00000")),
        Order(5, "IBM", Side::Sell, 10, Price("90.00000"))};
    std::unordered_map<OrderID, Order> orders;
    for (auto const &o : olist) {
      orders[o.id] = std::move(o);
    }
    std::vector<Result> results;
    hft::OrderMatcher matcher(orders, "IBM");
    matcher.add(0, results);
    matcher.add(1, results);
    matcher.add(2, results);
    matcher.add(3, results);
    matcher.add(4, results);
    CHECK_EMPTY(results);
    matcher.add(5, results);
    CHECK_EQUAL(results.size(), 2);
    for (auto const &r : results) {
      CHECK_EQUAL(r.type, ResultType::FillConfirm);
    }
    // std::cout << "results" << std::endl;
    // for ( auto & r : results ) {
    //   std::cout << r << std::endl;
    // }
    CHECK_EQUAL(results[0].order_id, 3);
    CHECK_EQUAL(results[0].quantity, 10);
    CHECK_EQUAL(results[0].price, Price("90.00000"));
    CHECK_EQUAL(results[1].order_id, 5);
    CHECK_EQUAL(results[1].quantity, 10);
    CHECK_EQUAL(results[1].price, Price("90.00000"));
  }

  /*
  ** ID Side Quantity Price
  ** 0  S    10       90.00000  -- X
  ** 1  S    100      120.00000
  ** 2  S    10       59.00000  X
  ** 3  S    100      100.00000
  ** 4  B    15       200.00000
  ** --->
  ** F 2 10 200  -- since 2 is the cheapest (full fill)
  ** F 0 5  200  -- second cheapest (partial fill)
  ** F 4 15 200  -- fully filled at bid price
  ** ======
  ** 5  B    1500     201.00000
  ** ---->
  ** F  0 5 201  -- full fill
  ** F  3 100 201 -- full fill
  ** F  1 100 201 -- full fill
  ** F  5 205 201 -- partial fill
  ** ======
  ** P
  ** ----->
  ** 5  B    1295     201.00000
   */
  {
    std::vector<Order> olist{
        Order(0, "IBM", Side::Sell, 10, Price("90.00000")),
        Order(1, "IBM", Side::Sell, 100, Price("120.00000")),
        Order(2, "IBM", Side::Sell, 10, Price("59.00000")),
        Order(3, "IBM", Side::Sell, 100, Price("100.00000")),
        Order(4, "IBM", Side::Buy, 15, Price("200.00000")),
        Order(5, "IBM", Side::Buy, 1500, Price("201.00000")),
    };
    std::unordered_map<OrderID, Order> orders;
    for (auto const &o : olist) {
      orders[o.id] = std::move(o);
    }
    std::vector<Result> results;
    hft::OrderMatcher matcher(orders, "IBM");
    matcher.add(0, results);
    matcher.add(1, results);
    matcher.add(2, results);
    matcher.add(3, results);
    matcher.add(4, results);
    CHECK_EQUAL(results.size(), 3);
    CHECK_EQUAL(results[0].order_id, 2);
    CHECK_EQUAL(results[1].order_id, 0);
    CHECK_EQUAL(results[2].order_id, 4);

    results.clear();
    matcher.add(5, results);
    CHECK_EQUAL(results.size(), 4);
    CHECK_EQUAL(results[0].order_id, 0);
    CHECK_EQUAL(results[1].order_id, 3);
    CHECK_EQUAL(results[2].order_id, 1);
    CHECK_EQUAL(results[3].order_id, 5);

    results.clear();
    matcher.print(results);
    CHECK_EQUAL(results.size(), 1);
    CHECK_EQUAL(results[0].order_id, 5);
    CHECK_EQUAL(results[0].quantity, 1295);

  }
  return true;
}

auto test_matcher() -> bool {
    // "O 10000 IBM B 10 100.00000"            | results.size() == 0
    // "O 10001 IBM B 10 99.00000"             | results.size() == 0
    // "O 10002 IBM S 5 101.00000"             | results.size() == 0
    // "O 10003 IBM S 5 100.00000"             | results.size() == 2
    // -->
    // "F 10003 IBM 5 100.00000"
    // "F 10000 IBM 5 100.00000"
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

    CHECK_EQUAL(results[0].order_id, 10000);
    CHECK_EQUAL(results[0].quantity, 5);
    CHECK_EQUAL(results[1].order_id, 10003);
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
    // std::cout << "add " << "B " << order_id << " " << 10 << " " << prices[i] << std::endl;
    book.add(Order(order_id++, "IBM", Side::Buy, 10, prices[i]));
  }
  // book.print();
  // std::cout << "results" << std::endl;
  // for (auto & r : book.getResults())
  //   std::cout << r << std::endl;

  for (auto i = 0; i < nitems; ++i) {
    auto sell_price = prices[i] - Price("1.00000");
    // std::cout << "add " << "S " << order_id << " " << sell_price << std::endl;
    book.add(Order(order_id++, "IBM", Side::Sell, 10, sell_price));
    // auto & r = book.getResults();
    // if (!r.empty()) {
    //   std::cout << "bought " << r[0].order_id << " at " << r[0].price << std::endl;
    // }
    // else {
    //   std::cout << "no match" << std::endl;
    // }
  }
  book.cancel(10000);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10001);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10003);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);
  book.cancel(10004);
  CHECK_EQUAL(book.getResults()[0].type, ResultType::CancelConfirm);

  // Due to highest bidder policy, the bids 19, 18, 17, 16, 15, 14 (ids 10, 9, 8, 7, 6, 5)
  // will be sold to fulfill (id 11, 12, 13, 14, 15, 16)
  // and the remaining orders (17, 18, 19, 20) will not be matched
  // the bids 1, 2, 3, 4 will not be matched

  book.print();
  // std::cout << "results" << std::endl;
  // for (auto & r : book.getResults())
  //   std::cout << r << std::endl;

  CHECK_EQUAL(book.getResults().size(), 8);
  auto res = book.getResults();
  std::sort(res.begin(), res.end(), [](Result const & a, Result const & b) {
    return a.order_id < b.order_id;
  });
  CHECK_EQUAL(res[0].order_id, 1);
  CHECK_EQUAL(res[1].order_id, 2);
  CHECK_EQUAL(res[2].order_id, 3);
  CHECK_EQUAL(res[3].order_id, 4);
  CHECK_EQUAL(res[4].order_id, 17);
  CHECK_EQUAL(res[5].order_id, 18);
  CHECK_EQUAL(res[6].order_id, 19);
  CHECK_EQUAL(res[7].order_id, 20);

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
  run_test(test_matcher, "Matcher");
  run_test(test_highest_bidder, "Highest bidder");
  run_test(test_cancellation, "Symbol book cancel");
  run_test(test_action, "Action");
  run_test(test_multi_symbol_book, "Multi symbol book");

  return 0;
}
