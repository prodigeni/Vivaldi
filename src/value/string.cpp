#include "value/string.h"

#include "gc.h"

using namespace il;

value::custom_type* value::string::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::string::value() const
{
  return '"' + m_val += '"';
}

value::base* value::string::copy() const
{
  return gc::alloc<string>( m_val );
}
