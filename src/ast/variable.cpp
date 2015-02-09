#include "variable.h"

#include "vm/instruction.h"

using namespace vv;

ast::variable::variable(symbol name) : m_name{name} { }

std::vector<vm::command> ast::variable::generate() const
{
  if (m_name == symbol{"self"})
    return { {vm::instruction::self} };
  return { {vm::instruction::read, m_name} };
}
