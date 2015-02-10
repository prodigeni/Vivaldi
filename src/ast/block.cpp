#include "block.h"

#include "vm/instruction.h"

using namespace vv;

ast::block::block(std::vector<std::unique_ptr<expression>>&& subexpressions)
  : m_subexpressions {move(subexpressions)}
{ }

std::vector<vm::command> ast::block::generate() const
{
  // Conceptually, *every* block statement consists of
  //   eblk
  //   push_nil
  //   expr_1
  //   expr_2
  //   ...
  //   expr_n
  //   lblk
  // But since the only time the push_nil is actually used is when there are no
  // expressions, and since in that case the e/lblk don't change any semantics,
  // there's no reason not to special-case it
  if (!m_subexpressions.size())
    return { {vm::instruction::push_nil} };

  std::vector<vm::command> vec{ {vm::instruction::eblk} };

  for (const auto& i : m_subexpressions) {
    auto subexpr = i->generate();
    copy(begin(subexpr), end(subexpr), back_inserter(vec));
  }

  vec.push_back(vm::instruction::lblk);
  return vec;
}
