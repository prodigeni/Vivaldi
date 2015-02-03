#include "floating_point.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::floating_point::floating_point(double value, environment& env)
  : base  {&builtin::type::floating_point, env},
    m_val {value}
{ }

std::string value::floating_point::value() const
{
  return std::to_string(m_val);
}

value::base* value::floating_point::copy() const
{
  return gc::alloc<floating_point>( m_val, *env().parent() );
}
