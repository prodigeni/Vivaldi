#include "return_statement.h"

#include "vm/instruction.h"

using namespace vv;

ast::return_statement::return_statement(std::unique_ptr<expression>&& value)
  : m_value {move(value)}
{ }

std::vector<vm::command> ast::return_statement::generate() const
{
  auto vec = m_value->generate();
  vec.emplace_back(vm::instruction::ret);
  return vec;
}
