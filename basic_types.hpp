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
      os << "F";
      break;
    case ResultType::CancelConfirm:
      os << "X";
      break;
    case ResultType::BookEntry:
      os << "P";
      break;
    case ResultType::Error:
      os << "E";
      break;
  }
  return os;
}


struct Result
{
  ResultType type;
  OrderID order_id;
  Symbol symbol;
  Quantity quantity;
  Price price;
  std::string_view error_message;

  static Result FillConfirm(OrderID id,  Symbol const & s, Quantity q, Price price)
  {
    return {ResultType::FillConfirm, id, s, q, price, ""};
  }
  static Result CancelConfirm(OrderID id, Symbol const & s)
  {
    return {ResultType::CancelConfirm, id, s, 0, Price(0), ""};
  }
  static Result Error(OrderID id, std::string_view error_message)
  {
    return {ResultType::Error, id, Symbol(), 0, Price(0), error_message};
  }
  static Result BookEntry(OrderID id, Symbol const &s, Quantity q, Price price)
  {
    return {ResultType::BookEntry, id, s, q, price, ""};
  }
};

std::ostream& operator<<(std::ostream& os, const Result& r) {
  os << r.type << " " << r.order_id;
  if (r.type == ResultType::FillConfirm) {
    os << " " << r.symbol << " " << r.quantity << " " << r.price;
  }
  else if (r.type == ResultType::CancelConfirm) {
    // os << " " << r.symbol;
  }
  else if (r.type == ResultType::Error) {
    os << " " << r.error_message;
  }
  else if (r.type == ResultType::BookEntry) {
    os << " " << r.symbol << " " << r.quantity << " " << r.price;
  }
  return os;
}

}  // end namespace hft
