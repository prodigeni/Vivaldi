#include "variable_declaration.h"

using namespace il;

ast::variable_declaration::variable_declaration(
    symbol name,
    std::unique_ptr<expression>&& value)
  : m_name  {name},
    m_value {move(value)}
{ }

value::base* ast::variable_declaration::eval(environment& env) const
{
  return env.create(m_name, m_value->eval(env));
}
