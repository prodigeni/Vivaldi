#include "floating_point.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::basic_type* value::floating_point::type() const
{
  return &builtin::type::floating_point;
}

std::string value::floating_point::value() const
{
  return std::to_string(m_val);
}

value::base* value::floating_point::copy() const
{
  return gc::alloc<floating_point>( m_val );
}
