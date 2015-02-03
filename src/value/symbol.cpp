#include "value/symbol.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::symbol::symbol(il::symbol val, environment& env)
  : base  {&builtin::type::symbol, env},
    m_val {val}
{ }

std::string value::symbol::value() const
{
  return '\'' + to_string(m_val);
}

value::base* value::symbol::copy() const
{
  return gc::alloc<symbol>( m_val, *env().parent() );
}
