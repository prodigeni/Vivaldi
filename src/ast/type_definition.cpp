#include "type_definition.h"

#include "value/custom_type.h"

using namespace il;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    std::vector<std::unique_ptr<expression>>&& public_mems,
    std::vector<std::unique_ptr<expression>>&& private_mems)
  : m_name            {name},
    m_parent          {parent},
    m_public_members  {move(public_mems)},
    m_private_members {move(private_mems)}
{ }

value::base* ast::type_definition::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
