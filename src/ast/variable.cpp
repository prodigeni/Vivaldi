#include "variable.h"

using namespace il;

ast::variable::variable(symbol name) : m_name{name} { }

value::base* ast::variable::eval(environment& env) const
{
  return env.at(m_name);
}
