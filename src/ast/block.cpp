#include "block.h"

#include "gc.h"

using namespace il;

ast::block::block(std::vector<std::unique_ptr<expression>>&& subexpressions)
  : m_subexpressions {move(subexpressions)}
{ }

std::vector<vm::command> ast::block::generate() const
{
  std::vector<vm::command> vec{ {vm::instruction::enter} };

  for (const auto& i : m_subexpressions) {
    auto subexpr = i->generate();
    copy(begin(subexpr), end(subexpr), back_inserter(vec));
  }

  vec.push_back(vm::instruction::leave);
  return vec;
}
