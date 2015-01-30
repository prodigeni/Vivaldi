#include "value/symbol.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::basic_type* value::symbol::type() const
{
  return &builtin::type::symbol;
}

std::string value::symbol::value() const
{
  return '\'' + to_string(m_val);
}

value::base* value::symbol::copy() const
{
  return gc::alloc<symbol>( m_val );
}
