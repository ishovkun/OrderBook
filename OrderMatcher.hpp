#pragma once
#include <map>
#include "basic_types.hpp"
#include <unordered_map>

namespace hft {

using hft::Order;
using hft::Result;

class OrderMatcher {

  std::unordered_map<OrderID, Order> & _orders;
  std::map<Price,std::vector<OrderID>> _buy;
  std::map<Price,std::vector<OrderID>> _sell;

 public:
  OrderMatcher(std::unordered_map<OrderID, Order> & orders)
      : _orders(orders)
  {}

  void add(OrderID iorder , std::vector<Result> & results);
  void cancel(OrderID iorder, std::vector<Result> & results);
  void print(std::vector<Result> & results) const;

 private:
  auto tryFill_(std::vector<Result> & results) -> void;
};

auto OrderMatcher::add(OrderID id, std::vector<Result> & results) -> void {
  if (!_orders.count(id)) {
    throw std::invalid_argument("Invalid order index");
  }

  auto & order = _orders[id];
  if (order.side == Side::Buy) {
    _buy[order.price].push_back(id);
  }
  else {
    _sell[order.price].push_back(id);
  }

  tryFill_(results);
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
  results.emplace_back(Result::CancelConfirm(id));
}


auto OrderMatcher::tryFill_(std::vector<Result> & results) -> void
{
  while (!_sell.empty() && !_buy.empty() && _sell.begin()->first <= _buy.rbegin()->first) {
    auto vector_sells = _sell.begin()->second;
    auto sell_id = _sell.begin()->second.front();
    auto & sell = _orders[sell_id];
    auto it_buy = _buy.lower_bound(sell.price);
    auto & vector_buys = it_buy->second;
    auto & buy = _orders[vector_buys.front()];

    Quantity fill_quantity = std::min(sell.quantity, buy.quantity);
    results.emplace_back(Result::FillConfirm(sell.id, fill_quantity, sell.price));
    results.emplace_back(Result::FillConfirm(buy.id, fill_quantity, sell.price));
    sell.quantity -= fill_quantity;
    buy.quantity -= fill_quantity;

    if (!sell.quantity) {
      vector_sells.erase(vector_sells.begin());
      if (vector_sells.empty()) {
        _sell.erase(_sell.begin());
      }
    }
    if (!buy.quantity) {
      vector_buys.erase(vector_buys.begin());
      if (vector_buys.empty()) {
        _buy.erase(it_buy);
      }
    }
  }
  return;
}

void OrderMatcher::print(std::vector<Result> & results) const
{
  for (auto const& container : {_sell, _buy}) {
    for (auto const & it : container) {
      for (auto order_id : it.second) {
        auto & order = _orders[order_id];
        results.emplace_back(Result::BookEntry(order_id, order.quantity, order.price) );
      }
    }
  }
}


}
