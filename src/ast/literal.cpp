#include "literal.h"

#include "gc.h"
#include "value/integer.h"

using namespace il;

ast::literal::literal(std::unique_ptr<value::base>&& value)
  : m_value {move(value)}
{ }

value::base* ast::literal::eval(environment& env) const
{
  return gc::alloc<value::integer>( *dynamic_cast<value::integer*>(m_value.get()) );
}
