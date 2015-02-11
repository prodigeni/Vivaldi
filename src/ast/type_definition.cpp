#include "type_definition.h"

#include "gc.h"

using namespace vv;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    std::unordered_map<vv::symbol,
                       std::unique_ptr<ast::function_definition>>&& methods)

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
  std::vector<vm::command> vec;

  vec.emplace_back(vm::instruction::push_sym, m_name);
  vec.emplace_back(vm::instruction::push_arg);

  vec.emplace_back(vm::instruction::read, m_parent);
  vec.emplace_back(vm::instruction::push_arg);

  for (const auto& i : m_methods) {
    auto definition = i.second->generate();
    copy(begin(definition), end(definition), back_inserter(vec));
    vec.emplace_back(vm::instruction::push_arg);

    vec.emplace_back(vm::instruction::push_sym, i.first);
    vec.emplace_back(vm::instruction::push_arg);
  }

  // Twice m_methods.size() (once through for names, once for function bodies),
  // plus one more for name and one for parent
  auto argc = static_cast<int>(2 + (m_methods.size() * 2));
  vec.emplace_back(vm::instruction::read, symbol{"Type"});
  vec.emplace_back(vm::instruction::call, argc);
  vec.emplace_back(vm::instruction::let, m_name);

  return vec;
}
