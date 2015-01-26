#include "literal.h"

using namespace il;

ast::literal::literal(std::unique_ptr<value::base>&& value)
  : m_value {move(value)}
{ }

value::base* ast::literal::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
