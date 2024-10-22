#pragma once
#include <array>
#include "memory"
#include "Price.hpp"
#include "Symbol.hpp"

namespace hft {

using OrderID = uint32_t;
using Quantity = uint16_t;

enum class Side : char { Buy = 'B', Sell = 'S' };

std::ostream& operator<<(std::ostream& os, const Side& s) {
  os << static_cast<char>(s);
  return os;
}

struct Order
{
  OrderID id;
  Symbol symbol;
  Side side;
  Quantity quantity;
  Price price;

  Order(OrderID id, Symbol symbol, Side side, Quantity quantity, Price price)
      : id(id), symbol(symbol), side(side), quantity(quantity), price(price)
  {}
  Order() = default;
};

std::ostream& operator<<(std::ostream& os, const Order& o) {
  os << o.id << " " << o.side << " " << o.quantity << " " << o.price;
  return os;
}

enum class ResultType
{
  FillConfirm,
  CancelConfirm,
  BookEntry,
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
    case ResultType::BookEntry:
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
  std::string_view error_message;

  static Result FillConfirm(OrderID id, Quantity q, Price price)
  {
    return {ResultType::FillConfirm, id, q, price, ""};
  }
  static Result CancelConfirm(OrderID id)
  {
    return {ResultType::CancelConfirm, id, 0, Price(0), ""};
  }
  static Result Error(OrderID id, std::string_view error_message)
  {
    return {ResultType::Error, id, 0, Price(0), error_message};
  }
  static Result BookEntry(OrderID id, Quantity q, Price price)
  {
    return {ResultType::BookEntry, id, q, price, ""};
  }
};

std::ostream& operator<<(std::ostream& os, const Result& r) {
  os << r.type << " " << r.order_id;
  if (r.type == ResultType::FillConfirm) {
    os << " " << r.quantity << " " << r.price;
  }
  else if (r.type == ResultType::CancelConfirm) {

  }
  else if (r.type == ResultType::Error) {
    os << " " << r.error_message;
  }
  else if (r.type == ResultType::BookEntry) {
    os << " " << r.quantity << " " << r.price;
  }
  return os;
}

}  // end namespace hft
