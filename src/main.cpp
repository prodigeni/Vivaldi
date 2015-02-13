#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "run_file.h"
#include "vm.h"
#include "value/builtin_function.h"
#include "value/nil.h"

#include <iostream>
#include <sstream>

void write_error(const std::string& error)
{
  std::cerr << "\033[1;31m" << error << "\033[22;39m\n";
}

void repl_catcher(vv::vm::machine& vm)
{
  write_error("caught exception: " + vm.retval->value());

  // Clear out remaining instructions once the current line's borked
  auto remaining = vm.stack->instr_ptr.size();
  vm.stack->instr_ptr = vm.stack->instr_ptr.subvec(remaining);
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
  while (!std::cin.eof()) {
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
  vv::gc::init();

  auto base_stack = std::make_shared<vv::vm::call_stack>(
      std::shared_ptr<vv::vm::call_stack>{},
      std::shared_ptr<vv::vm::call_stack>{},
      0,
      vv::vector_ref<vv::vm::command>{{}} );
  vv::builtin::make_base_env(*base_stack);

  while (!std::cin.eof()) {
    for (const auto& expr : get_valid_line()) {
      auto body = expr->generate();
      base_stack->instr_ptr = vv::vector_ref<vv::vm::command>{body};
      vv::vm::machine machine{base_stack, repl_catcher};
      machine.run();
      std::cout << "=> " << machine.retval->value() << '\n';
    }
  }
  std::cout << '\n'; // stick prompt on newline on ^D
}

int main(int argc, char** argv)
{
  if (argc > 2) {
    std::cerr << "Usage: " << argv[0] << " [file]\n";
    return 1;
  }

  vv::gc::init();

  if (argc == 1) {
    run_repl();
    vv::gc::empty();

  } else {
    auto ret = vv::run_file(argv[1]);

    if (ret.res == vv::run_file_result::result::file_not_found)
      std::cerr << argv[1] << ": file not found\n";
    else if (ret.res == vv::run_file_result::result::failure)
      std::cerr << "Caught exception: " << ret.val->value() << '\n';

    vv::gc::empty();
    return ret.res != vv::run_file_result::result::success;
  }
}
