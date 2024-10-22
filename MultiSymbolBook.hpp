#pragma once
#include <unordered_map>
#include "basic_types.hpp"
#include "OrderMatcher.hpp"

namespace hft {

class MultiSymbolBook {
  std::unordered_map<OrderID, Order> _orders;
  std::unordered_map<Symbol, OrderMatcher> _matchers;
  std::vector<Result> _results;

 public:
  MultiSymbolBook() = default;
  ~MultiSymbolBook() = default;

  void add(Order const &order) {
    _results.clear();
    if (_orders.count(order.id)) {
      _results.emplace_back(Result::Error(order.id, "Order already exists"));
      return;
    }
    _orders[order.id] = order;
    if (!_matchers.count(order.symbol)) {
      _matchers.emplace(order.symbol, OrderMatcher(_orders, order.symbol));
    }
    _matchers.find(order.symbol)->second.add(order.id, _results);

    for (auto const & result : _results) {
      if (result.type == ResultType::FillConfirm && _orders[result.order_id].quantity == 0) {
        _orders.erase(result.order_id);
      }
    }
  }

  std::vector<Result> const & getResults() {
    return _results;
  }

  void cancel(OrderID id) {
    _results.clear();
    if (!_orders.count(id)) {
      _results.emplace_back(Result::Error(id, "Order does not exist"));
    }
    else {
      auto &order = _orders[id];
      _matchers.find(order.symbol)->second.cancel(id, _results);
      _orders.erase(id);
    }
  }

  void print() {
    _results.clear();
    for (auto it = _matchers.begin(); it != _matchers.end(); ++it) {
      it->second.print(_results);
    }
  }

};



}  // end namespace hft
