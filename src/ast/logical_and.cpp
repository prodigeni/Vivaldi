#include "logical_and.h"

#include "vm/instruction.h"

using namespace vv;

ast::logical_and::logical_and(std::unique_ptr<expression>&& left,
                              std::unique_ptr<expression>&& right)
  : m_left  {move(left)},
    m_right {move(right)}
{ }

std::vector<vm::command> ast::logical_and::generate() const
{
  auto vec = m_left->generate();
  vec.emplace_back(vm::instruction::jmp_false);
  auto jmp_to_false_idx = vec.size() - 1;

  auto right = m_right->generate();
  copy(begin(right), end(right), back_inserter(vec));
  vec.emplace_back(vm::instruction::push_bool, true);
  vec.emplace_back(vm::instruction::jmp, 1);
  vec.emplace_back(vm::instruction::push_bool, false);

  vec[jmp_to_false_idx].arg = static_cast<int>(vec.size() - jmp_to_false_idx);
  return vec;
}
