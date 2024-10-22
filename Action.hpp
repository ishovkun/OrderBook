#pragma once
#include <expected>
#include "basic_types.hpp"
#include <sstream>

namespace hft {

enum class ActionType
{
  Place,
  Cancel,
  Print,
};

std::ostream& operator<<(std::ostream& os, const ActionType& o) {
  switch (o) {
    case ActionType::Place: os << "Place"; break;
    case ActionType::Cancel: os << "Cancel"; break;
    case ActionType::Print: os << "Print"; break;
  }
  return os;
}

struct Action
{
  ActionType type;
  Order order;
  Action(std::string const & s);
};

Action::Action(std::string const & s)
{
  std::stringstream ss(s);
  std::string type_str, symbol_str, side_str, quantity_str;
  ss >> type_str;

  if (type_str == "O") {
    type = ActionType::Place;
    ss >> order.id;
    ss >> order.symbol;
    ss >> side_str;
    // std::cout << "order.symbol = '" << order.symbol <<"'" << std::endl;
    if (side_str.size() != 1) {
      throw std::invalid_argument("Invalid side");
    }
    if (side_str[0] == 'B') {
      order.side = Side::Buy;
    }
    else if (side_str[0] == 'S') {
      order.side = Side::Sell;
    }
    else {
      throw std::invalid_argument("Invalid side");
    }
    ss >> order.quantity;
    ss >> order.price;
  }
  else if (type_str == "X") {
    type = ActionType::Cancel;
    ss >> order.id;
  }
  else if (type_str == "P") {
    type = ActionType::Print;
  }
  else {
    throw std::invalid_argument("Unknown action type");
  }

  std::string left;
  if (ss >> left) {
    throw std::invalid_argument("Invalid order");
  }
}


}  // end namespace hft
