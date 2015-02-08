#include "type_definition.h"

#include "gc.h"

using namespace il;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    const std::unordered_map<
              il::symbol,
              std::shared_ptr<ast::function_definition>>& m_methods)

  : m_name    {name},
    m_parent  {parent},
    m_methods {m_methods}
{ }

std::vector<vm::command> ast::type_definition::generate() const
{
  std::vector<vm::command> vec;
  for (const auto& i : m_methods) {
    auto definition = i.second->generate();
    vec.emplace_back(vm::instruction::push_fn, move(definition));
    vec.emplace_back(vm::instruction::push_arg);

    vec.emplace_back(vm::instruction::push_sym, i.first);
    vec.emplace_back(vm::instruction::push_arg);
  }

  // Twice m_methods.size(); once through for names, once for function bodies
  auto argc = static_cast<int>(m_methods.size() * 2);
  vec.emplace_back(vm::instruction::read, symbol{"Type"});
  vec.emplace_back(vm::instruction::call, argc);
  vec.emplace_back(vm::instruction::let, m_name);

  return vec;
}
