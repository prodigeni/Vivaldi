#include "integer.h"

#include "gc.h"
#include "builtins.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::basic_type* value::integer::type() const
{
  return &builtin::type::integer;
}

std::string value::integer::value() const { return std::to_string(m_val); }

value::base* value::integer::copy() const
{
  return gc::alloc<integer>( m_val );
}
