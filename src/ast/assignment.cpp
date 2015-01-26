#include "assignment.h"

using namespace il;

ast::assignment::assignment(symbol name, std::unique_ptr<expression>&& value)
  : m_name  {name},
    m_value {move(value)}
{ }

value::base* ast::assignment::eval(environment& env) const
{
  return env.at(m_name) = m_value->eval(env);
}
