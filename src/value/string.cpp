#include "value/string.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::basic_type* value::string::type() const
{
  return &builtin::type::string;
}

std::string value::string::value() const
{
  return '"' + m_val += '"';
}

value::base* value::string::copy() const
{
  return gc::alloc<string>( m_val );
}
