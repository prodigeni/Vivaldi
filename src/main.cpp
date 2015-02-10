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

void repl_catcher(vv::vm::machine& vm)
{
  write_error("caught exception: " + vm.retval->value());

  // Clear out remaining instructions once the current line's borked
  auto remaining = static_cast<long>(vm.stack->instr_ptr.size());
  vm.stack->instr_ptr = vm.stack->instr_ptr.remove_prefix(remaining);
  vm.retval = vv::gc::alloc<vv::value::nil>( );
}

void normal_catcher(vv::vm::machine& vm)
{
  std::cerr << "caught exception: " << vm.retval->value() << '\n';
  vv::gc::empty();
  exit(0);
}

std::vector<std::unique_ptr<vv::ast::expression>> get_valid_line()
{
  std::cout << ">>> ";
  std::vector<std::string> tokens;
  vv::parser::val_res validator;
  for (;;) {
    std::string line;
    getline(std::cin, line);
    std::istringstream linestream{line};

    auto new_tokens = vv::parser::tokenize(linestream);
    copy(begin(new_tokens), end(new_tokens), back_inserter(tokens));
    validator = vv::parser::is_valid(tokens);
    if (validator.valid())
      break;

    if (validator.valid() || validator->size()) {
      std::string error{"invalid syntax"};
      if (validator.invalid())
        error += " at "
              + (validator->front() == "\n"
                  ? "end of line: "
                  : '\'' + validator->front() + "': ")
              + validator.error();
      write_error(error);
      tokens.clear();
      std::cout << ">>> ";
    } else {
      std::cout << "... ";
    }
  }

  return vv::parser::parse(tokens);
}

void run_repl()
{
  auto base_stack = std::make_shared<vv::vm::call_stack>(
      std::shared_ptr<vv::vm::call_stack>{},
      std::shared_ptr<vv::vm::call_stack>{},
      std::vector<vv::value::base*>{},
      vv::vector_ref<vv::vm::command>{{}} );
  vv::builtin::make_base_env(*base_stack);

  for (;;) {
    for (const auto& expr : get_valid_line()) {
      auto body = expr->generate();
      base_stack->instr_ptr = vv::vector_ref<vv::vm::command>{body};
      vv::vm::machine machine{base_stack, repl_catcher};
      machine.run();
      std::cout << "=> " << machine.retval->value() << '\n';
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
  vv::vm::machine machine{base_stack, normal_catcher};
  machine.run();

  vv::gc::empty();
}
