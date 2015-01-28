#include "repl.h"

#include "parser.h"
#include "expression.h"
#include "value.h"

#include <iostream>
#include <sstream>

void error(const std::string& message)
{
  std::cerr << "\033[91m" << message << "\033[39m\n";
}

void il::run_repl()
{
  environment env{};
  std::string cur_line;
  do {
    std::cout << "il> ";
    getline(std::cin, cur_line);
    std::stringstream stream{cur_line};
    auto tokens = parser::tokenize(stream);
    if (parser::is_valid(tokens)) {
      auto expr = parser::parse(tokens);
      try {
        for (const auto& i : expr)
          std::cout << "=> " << i->eval(env)->value() << '\n';
      } catch (const std::runtime_error& err) {
        error(err.what());
      }
    } else {
      error("Invalid syntax");
    }
  } while (!std::cin.eof());
  std::cout << '\n';
};
