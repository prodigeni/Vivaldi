#include "logical_or.h"

#include "vm/instruction.h"

using namespace vv;

ast::logical_or::logical_or(std::unique_ptr<expression>&& left,
                              std::unique_ptr<expression>&& right)
  : m_left  {move(left)},
    m_right {move(right)}
{ }

std::vector<vm::command> ast::logical_or::generate() const
{
  // Given conditions 'a' and 'b', generate the following VM instructions:
  //   a
  //   jmp_true <true index - this index>
  //   b
  //   jmp_true 3
  //   push_bool false
  //   jmp 2
  //   push_bool true
  auto vec = m_left->generate();
  vec.emplace_back(vm::instruction::jmp_true);
  auto jmp_to_false_idx = vec.size() - 1;

  auto right = m_right->generate();
  copy(begin(right), end(right), back_inserter(vec));
  vec.emplace_back(vm::instruction::jmp_true, 3);
  vec.emplace_back(vm::instruction::push_bool, false);
  vec.emplace_back(vm::instruction::jmp, 2);
  vec.emplace_back(vm::instruction::push_bool, true);

  auto false_idx = vec.size() - 1;
  vec[jmp_to_false_idx].arg = static_cast<int>(false_idx - jmp_to_false_idx);
  return vec;
}
