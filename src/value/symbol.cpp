#include "value/symbol.h"

#include "gc.h"

using namespace il;

value::basic_type* value::symbol::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::symbol::value() const
{
  return '\'' + to_string(m_val);
}

value::base* value::symbol::copy() const
{
  return gc::alloc<symbol>( m_val );
}
