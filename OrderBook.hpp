#pragma once
#include <map>
#include "basic_types.hpp"

namespace hft {

class OrderBook {
  std::map<Price,std::vector<SymbolOrder>> _buy;
  std::map<Price,std::vector<SymbolOrder>> _sell;

 public:
  OrderBook() = default;

  int add(SymbolOrder const & order, std::vector<Result> & results);
  int cancel(OrderID id, std::vector<Result> & results);
  int print(std::vector<Result> & results) const;

 private:
  bool contains(OrderID id) const;
  int tryFill_(std::vector<Result> & results);
};

bool OrderBook::contains(OrderID id) const
{
  for (auto & container : {_buy, _sell}) {
    for (auto it = container.begin(); it != container.end(); ++it) {
      for (auto & order : it->second) {
        if (order.id == id) {
          return true;
        }
      }
    }
  }
  return false;
}

int OrderBook::add(SymbolOrder const & order, std::vector<Result> & results) {
  if (contains(order.id)) {
    results.emplace_back(Result::Error(order.id, "Duplicate order id"));
  }
  if (order.side == Side::Buy) {
    _buy[order.price].push_back(order);
  }
  else {
    _sell[order.price].push_back(order);
  }
  return tryFill_(results);
}

int OrderBook::tryFill_(std::vector<Result> & results)
{
  size_t const old_size = results.size();
  while (!_sell.empty() && !_buy.empty() && _sell.begin()->first <= _buy.rbegin()->first) {
    auto vector_sells = _sell.begin()->second;
    auto & sell = _sell.begin()->second.front();
    auto it_buy = _buy.lower_bound(sell.price);
    auto & vector_buys = it_buy->second;
    auto & buy = vector_buys.front();

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
  return results.size() - old_size;
}

int OrderBook::cancel(OrderID id, std::vector<Result> & results) {
  for (auto & container : {_buy, _sell}) {
    for (auto it = container.begin(); it != container.end(); ++it) {
      for (auto & order : it->second) {
        if (order.id == id) {
          results.emplace_back(Result::CancelConfirm(id));
          return 1;
        }
      }
    }
  }
  results.emplace_back(Result::Error(id, "Order does not exist"));
  return 1;
}

}  // end namespace hft
