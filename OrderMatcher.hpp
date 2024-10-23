#pragma once
#include <map>
#include "basic_types.hpp"
#include <unordered_map>

namespace hft {

using hft::Order;
using hft::Result;

class OrderMatcher {

  std::unordered_map<OrderID, Order> & _orders;
  std::map<Price,std::vector<OrderID>, std::greater<Price>> _buy;
  std::map<Price,std::vector<OrderID>> _sell;
  Symbol _symbol;

 public:
  OrderMatcher(std::unordered_map<OrderID, Order> & orders, Symbol symbol)
      : _orders(orders), _symbol(symbol)
  {}

  void add(OrderID iorder , std::vector<Result> & results);
  void cancel(OrderID iorder, std::vector<Result> & results);
  void print(std::vector<Result> & results) const;

 private:
  auto tryBuy_(Order & buy, std::vector<Result> & results) -> void;
  auto trySell_(Order & sell, std::vector<Result> & results) -> void;
};

auto OrderMatcher::add(OrderID id, std::vector<Result> & results) -> void {
  if (!_orders.count(id)) {
    throw std::invalid_argument("Invalid order index");
  }

  auto & order = _orders[id];
  if (order.side == Side::Buy) {
    tryBuy_(order, results);
    if (order.quantity) {
      _buy[order.price].push_back(id);
    }
  }
  else {
    trySell_(order, results);
    if (order.quantity) {
      _sell[order.price].push_back(id);
    }
  }
}

void OrderMatcher::cancel(OrderID id, std::vector<Result> & results)
{
  if (!_orders.count(id)) {
    results.emplace_back(Result::Error(id, "Order does not exist"));
    return;
  }
  auto & order = _orders[id];
  if (order.side == Side::Buy) {
    auto & buys_with_price = _buy[order.price];
    auto it = std::find(buys_with_price.begin(), buys_with_price.end(), id);
    buys_with_price.erase(it);
    if (buys_with_price.empty()) {
      _buy.erase(order.price);
    }
  }
  else {
    auto & sells_with_price = _sell[order.price];
    auto it = std::find(sells_with_price.begin(), sells_with_price.end(), id);
    sells_with_price.erase(it);
    if (sells_with_price.empty()) {
      _sell.erase(order.price);
    }
  }
  results.emplace_back(Result::CancelConfirm(id, _symbol));
}

void OrderMatcher::print(std::vector<Result> & results) const
{
  // Cannot do those in a single loo for (auto & container : {_buy, _sell})
  // because those maps use different comparators
  for (auto const & it : _buy) {
    for (auto order_id : it.second) {
      auto & order = _orders[order_id];
      results.emplace_back(Result::BookEntry(order_id, _symbol, order.quantity, order.price) );
    }
  }
  for (auto const & it : _sell) {
    for (auto order_id : it.second) {
      auto & order = _orders[order_id];
      results.emplace_back(Result::BookEntry(order_id, _symbol, order.quantity, order.price) );
    }
  }
}

auto OrderMatcher::tryBuy_(Order &buy, std::vector<Result> &results) -> void
{
  /*
  ** 1. First-in-First-Out (FIFO)
  ** FIFO is also known as a price-time algorithm.
  ** According to the FIFO algorithm, buy orders take priority in the order of price and time.
  ** Then, buy orders with the same maximum price are prioritized based on the time of bid,
  ** and priority is given to the first buy order.
  ** It is automatically prioritized over the buy orders at lower prices.
  **
  ** For example, a buy order for 300 shares of a security at $50 per share is followed by another
  ** buy order of 100 shares of the same security at a similar price.
  ** According to the FIFO algorithm, the total 300 shares buy order will be matched to sell orders.
  ** There can be more than one sell order. After the 300 shares buy order is matched,
  ** the 100 shares buy order matching will start.
  */
  auto old_quantity = buy.quantity;
  while (buy.quantity && !_sell.empty() && buy.price >= _sell.begin()->first) {
    auto &cheapest_sells = _sell.begin()->second;
    auto earliest_sell = cheapest_sells.front();
    auto &sell = _orders[earliest_sell];

    Quantity fill_quantity = std::min(sell.quantity, buy.quantity);
    results.emplace_back(Result::FillConfirm(sell.id, _symbol, fill_quantity, buy.price));
    sell.quantity -= fill_quantity;
    buy.quantity -= fill_quantity;

    if (!sell.quantity) {
      cheapest_sells.erase(cheapest_sells.begin());
      if (cheapest_sells.empty()) {
        _sell.erase(_sell.begin());
      }
    }
  }
  if (buy.quantity < old_quantity) {
    results.emplace_back(Result::FillConfirm(buy.id, _symbol, old_quantity - buy.quantity, buy.price));
  }
}

auto OrderMatcher::trySell_(Order &sell, std::vector<Result> &results) -> void
{
  auto old_quantity = sell.quantity;
  while (sell.quantity && !_buy.empty() && sell.price <= _buy.begin()->first) {
    auto & highest_buys = _buy.begin()->second;
    auto earliest_buy = highest_buys.front();
    auto & buy = _orders[earliest_buy];

    Quantity fill_quantity = std::min(sell.quantity, buy.quantity);
    results.emplace_back(Result::FillConfirm(buy.id, _symbol, fill_quantity, sell.price));
    sell.quantity -= fill_quantity;
    buy.quantity -= fill_quantity;

    if (!buy.quantity) {
      highest_buys.erase(highest_buys.begin());
      if (highest_buys.empty()) {
        _buy.erase(_buy.begin());
      }
    }
  }

  if (sell.quantity < old_quantity) {
    results.emplace_back(Result::FillConfirm(sell.id, _symbol, old_quantity - sell.quantity, sell.price));
  }
}


}
