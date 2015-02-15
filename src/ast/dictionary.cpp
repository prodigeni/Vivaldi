#include "dictionary.h"

#include "vm/instruction.h"

using namespace vv;

ast::dictionary::dictionary(std::vector<std::unique_ptr<ast::expression>>&& members)
  : m_members  {move(members)}
{ }

std::vector<vm::command> ast::dictionary::generate() const
{
  std::vector<vm::command> vec;

  for (const auto& i : m_members) {
    auto arg = i->generate();
    copy(begin(arg), end(arg), back_inserter(vec));
    vec.emplace_back(vm::instruction::push);
  }

  vec.emplace_back(vm::instruction::make_dict,
                   static_cast<int>(m_members.size()));
  return vec;
}
