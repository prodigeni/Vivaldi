#include "type_definition.h"

#include "gc.h"

using namespace il;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    const std::vector<symbol>& public_mems,
    const std::unordered_map<
              il::symbol,
              std::shared_ptr<ast::function_definition>>& m_methods)

  : m_name    {name},
    m_parent  {parent},
    m_members {public_mems},
    m_methods {m_methods}
{ }

std::vector<vm::command> ast::type_definition::generate() const
{
  throw std::runtime_error{"not yet implemented"};
}
