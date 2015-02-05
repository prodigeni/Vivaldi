#include "assignment.h"

#include "gc.h"

using namespace il;

ast::assignment::assignment(symbol name, std::unique_ptr<expression>&& value)
  : m_name  {name},
    m_value {move(value)}
{ }

value::base* ast::assignment::eval(environment& env) const
{
  auto val = gc::push_argument(m_value->eval(env));
  env.assign(m_name, val);
  gc::pop_argument();
  return val;
}
