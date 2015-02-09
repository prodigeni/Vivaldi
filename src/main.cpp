#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "vm.h"
#include "value/builtin_function.h"
#include "value/nil.h"

#include <iostream>
#include <fstream>
#include <sstream>

void write_error(const std::string& error)
{
  std::cerr << "\033[1;31m" << error << "\033[22;39m\n";
}

vv::value::base* repl_catcher(vv::vm::machine& vm)
{
  write_error("caught exception: " + vm.stack->args.front()->value());
  return vv::gc::alloc<vv::value::nil>( );
}

void run_repl()
{
  vv::value::builtin_function catcher{repl_catcher};

  auto base_stack = std::make_shared<vv::vm::call_stack>(
      std::shared_ptr<vv::vm::call_stack>{},
      std::shared_ptr<vv::vm::call_stack>{},
      std::vector<vv::value::base*>{},
      vv::vector_ref<vv::vm::command>{{}} );
  vv::builtin::make_base_env(*base_stack);

  base_stack->catchers.push_back(&catcher);

  for (;;) {
    std::cout << ">>> ";
    std::string line;
    getline(std::cin, line);
    std::istringstream linestream{line};
    auto tokens = vv::parser::tokenize(linestream);
    if (!vv::parser::is_valid(tokens)) {
      write_error("invalid syntax");
    } else {
      auto exprs = vv::parser::parse(tokens);
      for (const auto& expr : exprs) {
        auto body = expr->generate();
        base_stack->instr_ptr = vv::vector_ref<vv::vm::command>{body};
        vv::vm::machine machine{base_stack};
        machine.run();
        std::cout << "=> " << machine.retval->value() << '\n';
      }
    }
  }
}

int main(int argc, char** argv)
{
  if (argc == 1) {
    run_repl();
    return 0;
  }
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  std::ifstream file{argv[1]};
  auto tokens = vv::parser::tokenize(file);
  auto exprs = vv::parser::parse(tokens);
  std::vector<vv::vm::command> body;
  for (const auto& i : exprs) {
    auto code = i->generate();
    copy(begin(code), end(code), back_inserter(body));
  }
  auto base_stack = std::make_shared<vv::vm::call_stack>(
      std::shared_ptr<vv::vm::call_stack>{},
      std::shared_ptr<vv::vm::call_stack>{},
      std::vector<vv::value::base*>{},
      vv::vector_ref<vv::vm::command>{body} );

  vv::builtin::make_base_env(*base_stack);
  vv::vm::machine machine{base_stack};
  machine.run();

  vv::gc::empty();
}
