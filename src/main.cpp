#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "repl.h"
#include "vm/call_stack.h"
#include "vm/instruction.h"

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
  std::vector<il::vm::command> body;
  for (const auto& i : exprs) {
    auto code = i->generate();
    copy(begin(code), end(code), back_inserter(body));
  }
  auto base_stack = std::make_shared<il::vm::call_stack>(
      std::shared_ptr<il::vm::call_stack>{},
      std::shared_ptr<il::vm::call_stack>{},
      std::vector<il::value::base*>{},
      il::vector_ref<il::vm::command>{body} );

  il::gc::empty();
}
