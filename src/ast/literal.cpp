#include "literal.h"

#include "vm/instruction.h"

using namespace vv;

std::vector<vm::command> ast::literal::boolean::generate() const
{
  return { {vm::instruction::push_bool, m_val} };
}

std::vector<vm::command> ast::literal::floating_point::generate() const
{
  return { {vm::instruction::push_flt, m_val} };
}

std::vector<vm::command> ast::literal::integer::generate() const
{
  return { {vm::instruction::push_int, m_val} };
}

std::vector<vm::command> ast::literal::nil::generate() const
{
  return { {vm::instruction::push_nil} };
}

std::vector<vm::command> ast::literal::string::generate() const
{
  return { {vm::instruction::push_str, m_val} };
}

std::vector<vm::command> ast::literal::symbol::generate() const
{
  return { {vm::instruction::push_sym, m_val} };
}
