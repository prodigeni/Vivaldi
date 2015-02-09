#include "member.h"

#include "vm/instruction.h"

using namespace vv;

ast::member::member(std::unique_ptr<ast::expression>&& object, vv::symbol name)
  : m_object {move(object)},
    m_name   {name}
{ }

std::vector<vm::command> ast::member::generate() const
{
  auto vec = m_object->generate();
  vec.emplace_back(vm::instruction::readm, m_name);
  return vec;
}
