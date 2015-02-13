#include "type_definition.h"

#include "gc.h"
#include "vm/instruction.h"

#include <boost/variant/get.hpp>

using namespace vv;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    std::unordered_map<vv::symbol, ast::function_definition>&& methods)

  : m_name    {name},
    m_parent  {parent},
    m_methods {move(methods)}
{ }

std::vector<vm::command> ast::type_definition::generate() const
{
  // This is ABSOLUTELY HIDEOUS. Instead of doing it the right way and having a
  // builtin type-definition instruction, I create types by calling the Type
  // constructor with the following function arguments:
  // 1. a type name
  // 1. a parent class
  // 2. a method name
  // 3. a method body
  // (where 2 and 3 are repeated for each method given)
  // At some point I should probably do not that, but it does *work*
  std::unordered_map<symbol, std::vector<vm::command>> methods;
  for (const auto& i : m_methods) {
    auto arg = i.second.generate().front().arg;
    methods[i.first] = boost::get<std::vector<vm::command>>(arg);
  }

  std::vector<vm::command> vec;
  vec.emplace_back(vm::instruction::push_type,
                   vm::type_t{m_name, m_parent, methods});
  return vec;
}
