#include "object_creation.h"

#include "vm/instruction.h"

using namespace vv;

ast::object_creation::object_creation(
    std::unique_ptr<ast::expression>&& type,
    std::vector<std::unique_ptr<ast::expression>>&& args)
  : m_type {move(type)},
    m_args {move(args)}
{ }

std::vector<vm::command> ast::object_creation::generate() const
{
  std::vector<vm::command> vec;

  for (const auto& i : m_args) {
    auto arg = i->generate();
    copy(begin(arg), end(arg), back_inserter(vec));
    vec.emplace_back(vm::instruction::push_arg);
  }

  auto type = m_type->generate();
  copy(begin(type), end(type), back_inserter(vec));

  vec.emplace_back(vm::instruction::new_obj, static_cast<int>(m_args.size()));
  return vec;
}
