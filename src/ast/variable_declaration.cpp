#include "variable_declaration.h"

#include "vm/instruction.h"

using namespace il;

ast::variable_declaration::variable_declaration(
    symbol name,
    std::unique_ptr<expression>&& value)
  : m_name  {name},
    m_value {move(value)}
{ }

std::vector<vm::command> ast::variable_declaration::generate() const
{
  auto vec = m_value->generate();
  vec.emplace_back(vm::instruction::let, m_name);
  return vec;
}
