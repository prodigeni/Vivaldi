#include "value/string.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::string::string(const std::string& val, environment& env)
  : base  {&builtin::type::string, env},
    m_val {val}
{ }

std::string value::string::value() const
{
  return '"' + m_val += '"';
}

value::base* value::string::copy() const
{
  return gc::alloc<string>( m_val, *env().parent() );
}
