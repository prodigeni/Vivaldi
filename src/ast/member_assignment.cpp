#include "member_assignment.h"

#include "vm/instruction.h"

using namespace vv;

ast::member_assignment::member_assignment(
    std::unique_ptr<ast::expression>&& object,
    vv::symbol name,
    std::unique_ptr<ast::expression>&& value)
  : m_object {move(object)},
    m_name   {name},
    m_value  {move(value)}
{ }

std::vector<vm::command> ast::member_assignment::generate() const
{
  auto vec = m_value->generate();
  vec.emplace_back(vm::instruction::push);
  auto obj = m_object->generate();
  copy(begin(obj), end(obj), back_inserter(vec));
  vec.emplace_back(vm::instruction::writem, m_name);

  return vec;
}
