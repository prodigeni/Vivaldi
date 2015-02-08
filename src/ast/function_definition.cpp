#include "function_definition.h"

#include "gc.h"
#include "value/function.h"

using namespace il;

ast::function_definition::function_definition(symbol name,
                                              std::unique_ptr<expression>&&body,
                                              const std::vector<symbol>& args)
  : m_name {name},
    m_body {move(body)},
    m_args {args}
{ }

std::vector<vm::command> ast::function_definition::generate() const
{
  std::vector<vm::command> definition;
  for (auto i = rbegin(m_args); i != rend(m_args); ++i)
    definition.emplace_back(vm::instruction::pop_arg, *i);

  auto body = m_body->generate();
  copy(begin(body), end(body), back_inserter(definition));
  definition.emplace_back(vm::instruction::ret);

  std::vector<vm::command> vec;
  vec.emplace_back(vm::instruction::push_fn, move(definition));

  const static symbol nonname{""};
  if (m_name != nonname)
    vec.emplace_back(vm::instruction::let, m_name);

  return vec;
}
