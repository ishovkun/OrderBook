#pragma once
#include <array>
#include "Price.hpp"
#include "memory"

namespace hft {

using OrderID = uint32_t;
using Symbol = std::array<char, 8>;
using Quantity = uint16_t;

enum class ActionType
{
  Place,
  Cancel,
  Print,
};

enum class Side : char { Buy = 'B', Sell = 'S' };

std::ostream& operator<<(std::ostream& os, const Side& s) {
  os << static_cast<char>(s);
  return os;
}

struct SymbolOrder
{
  OrderID id;
  Side side;
  Quantity quantity;
  Price price;

  SymbolOrder(OrderID id, Side side, Quantity quantity, Price price)
      : id(id), side(side), quantity(quantity), price(price)
  {}
};

std::ostream& operator<<(std::ostream& os, const SymbolOrder& o) {
  os << o.id << " " << o.side << " " << o.quantity << " " << o.price;
  return os;
}

struct Order
{
  Symbol symbol;
  SymbolOrder payload;
};

struct Action
{
  ActionType type;
  OrderID id;
  std::unique_ptr<Order> order{nullptr};
};

enum class ResultType
{
  FillConfirm,
  CancelConfirm,
  Print,
  Error,
};

std::ostream& operator<<(std::ostream& os, ResultType type) {
  switch (type) {
    case ResultType::FillConfirm:
      os << "FillConfirm";
      break;
    case ResultType::CancelConfirm:
      os << "CancelConfirm";
      break;
    case ResultType::Print:
      os << "Print";
      break;
    case ResultType::Error:
      os << "Error";
      break;
  }
  return os;
}


struct Result
{
  ResultType type;
  OrderID order_id;
  Quantity quantity;
  Price price;
  std::string error_message;

  static Result FillConfirm(OrderID id, Quantity q, Price price)
  {
    return {ResultType::FillConfirm, id, q, price, ""};
  }
  static Result CancelConfirm(OrderID id)
  {
    return {ResultType::CancelConfirm, id, 0, Price(0), ""};
  }
  static Result Error(OrderID id, std::string const & error_message)
  {
    return {ResultType::Error, id, 0, Price(0), error_message};
  }
};

}  // end namespace hft
