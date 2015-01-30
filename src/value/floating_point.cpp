#include "floating_point.h"

#include "gc.h"

#include <string>

using namespace il;

value::basic_type* value::floating_point::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::floating_point::value() const
{
  return std::to_string(m_val);
}

value::base* value::floating_point::copy() const
{
  return gc::alloc<floating_point>( m_val );
}
