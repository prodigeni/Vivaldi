#include "run_file.h"

#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "vm.h"
#include "value/string.h"

#include <boost/filesystem.hpp>

#include <fstream>

namespace {

std::string message_for(vv::vector_ref<std::string> tokens,
                        const std::string& filename,
                        vv::parser::val_res validator)
{
  if (validator.invalid()) {
    const std::string* first{tokens.data()};
    auto line = count(first, begin(*validator), "\n");
    auto token = validator->size() == 0 ? "end of input"
                                        : '\'' + validator->front() + '\'';
    return "Invalid syntax at " + token
           + " in " + filename + " on line " + std::to_string(line + 1) + ": "
           + validator.error();
  }

  return "Invalid syntax in " + filename;
}

}

vv::run_file_result vv::run_file(const std::string& filename)
{
  std::ifstream file{filename};
  if (!file)
    return { run_file_result::result::file_not_found,
             gc::alloc<value::string>( '"' + filename + "\": file not found" ),
             {} };

  auto tokens = parser::tokenize(file);
  auto validator = parser::is_valid(tokens);
  if (!validator)
    return { run_file_result::result::failure,
             gc::alloc<value::string>(message_for(tokens, filename, validator)),
             {} };

  auto exprs = vv::parser::parse(tokens);
  std::vector<vv::vm::command> body;
  for (const auto& i : exprs) {
    auto code = i->generate();
    copy(begin(code), end(code), back_inserter(body));
  }

  // set working directory to path of file
  auto pwd = boost::filesystem::current_path();
  boost::filesystem::path path{filename};
  if (path.has_parent_path())
    boost::filesystem::current_path(path.parent_path());

  // Set up base env
  auto vm_base = std::make_shared<vm::call_frame>(nullptr, nullptr, 0, body);
  builtin::make_base_env(*vm_base);

  // Flag to check for exceptions--- slightly hacky, but oh well
  auto excepted = false;
  vm::machine machine{vm_base, [&](vm::machine&) { excepted = true; }};
  machine.run();

  // reset working directory
  boost::filesystem::current_path(pwd);

  if (excepted)
    return { run_file_result::result::failure, machine.retval, {} };

  return { run_file_result::result::success,
           gc::alloc<value::boolean>( true ),
           vm_base->local.front() };
}
