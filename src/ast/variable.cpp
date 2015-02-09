#include "variable.h"

#include "vm/instruction.h"

using namespace vv;

ast::variable::variable(symbol name) : m_name{name} { }

std::vector<vm::command> ast::variable::generate() const
{
  return { {vm::instruction::read, m_name} };
}
