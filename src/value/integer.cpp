#include "integer.h"

#include "gc.h"
#include "builtins.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::integer::integer(int val, environment& env)
  : base  {&builtin::type::integer, env},
    m_val {val}
{ }

std::string value::integer::value() const { return std::to_string(m_val); }

value::base* value::integer::copy() const
{
  return gc::alloc<integer>( m_val, *env().parent() );
}
