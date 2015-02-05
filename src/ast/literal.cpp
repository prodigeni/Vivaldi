#include "literal.h"

#include "gc.h"

using namespace il;

ast::literal::literal(std::unique_ptr<value::base>&& value)
  : m_value {value.release()}
{
  gc::push_ast(m_value);
}

value::base* ast::literal::eval(environment&) const
{
  return m_value->copy();
}

ast::literal::~literal()
{
  gc::pop_ast(m_value);
}
