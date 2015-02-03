#include "boolean.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::boolean::boolean(bool val, environment& env)
  : base  {&builtin::type::boolean, env},
    m_val {val}
{ }

std::string value::boolean::value() const
{
  return m_val ? "true" : "false";
}

value::base* value::boolean::copy() const
{
  return gc::alloc<boolean>( m_val, *env().parent() );
}
