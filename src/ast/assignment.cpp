#include "assignment.h"

#include "vm/instruction.h"

using namespace vv;

ast::assignment::assignment(symbol name, std::unique_ptr<expression>&& value)
  : m_name  {name},
    m_value {move(value)}
{ }

std::vector<vm::command> ast::assignment::generate() const
{
  auto vec = m_value->generate();
  vec.emplace_back(vm::instruction::write, m_name);
  return vec;
}
