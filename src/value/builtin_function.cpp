#include "builtin_function.h"

#include "gc.h"

using namespace il;

value::builtin_function::builtin_function(
    std::function<base*(const std::vector<base*>& args)> body)
  : m_body {body}
{ }

value::basic_type* value::builtin_function::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}

value::base* value::builtin_function::call(const std::vector<base*>& args)
{
  return m_body(args);
}

value::base* value::builtin_function::copy() const
{
  return gc::alloc<builtin_function>( m_body );
}
