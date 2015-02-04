#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "repl.h"

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  if (argc > 2) {
    std::cerr << "Usage: " << argv[0] << " [file]\n";
    return 1;
  }
  if (argc == 1) {
    il::run_repl();
    return 0;
  }

  std::ifstream file{argv[1]};
  auto tokens = il::parser::tokenize(file);
  auto exprs = il::parser::parse(tokens);
  for (const auto& i : exprs)
    i->eval(il::builtin::g_base_env);
  il::gc::empty();
}
