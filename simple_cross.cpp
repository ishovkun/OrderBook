// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include "MultiSymbolBook.hpp"
#include "Action.hpp"

typedef std::list<std::string> results_t;

class SimpleCross
{
  hft::MultiSymbolBook _book;
public:
    results_t action(const std::string& line) {
      try {
        hft::Action a(line);
        switch (a.type) {
          case hft::ActionType::Place : {
            _book.add(a.order);
            break;
          }
         case hft::ActionType::Cancel : {
            _book.cancel(a.order.id);
            break;
          }
          case hft::ActionType::Print : {
            _book.print();
            break;
          }
            default:
              return results_t{"Unknown action type"};
        }
        return toString(_book.getResults());
      }
      catch (std::exception const & e) {
        return results_t{e.what()};
      }
    }
 private:
  results_t toString(std::vector<hft::Result> const & results) {
    results_t res;
    for (auto const & r : results) {
      std::ostringstream os;
      os << r;
      res.push_back(os.str());
    }
    return res;
  }
};

// int main(int argc, char **argv)
int main()
{
  SimpleCross scross;
  std::string line;
  std::ifstream actions("actions.txt", std::ios::in);
  while (std::getline(actions, line)) {
    if (line.empty()) continue;

    // std::cout << "entry " << line << std::endl;
    results_t results = scross.action(line);
    // std::cout << "\t\t\tresults.size() = " << results.size() << std::endl;
    for (results_t::const_iterator it = results.begin(); it != results.end(); ++it) {
      std::cout << *it << std::endl;
    }
  }
  return 0;
}

