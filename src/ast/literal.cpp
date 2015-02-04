#include "literal.h"

#include "gc.h"
#include "value/integer.h"

using namespace il;

ast::literal::literal(std::unique_ptr<value::base>&& value)
  : m_value {move(value)}
{ }

value::base* ast::literal::eval(environment&) const
{
  return m_value.get();
}
