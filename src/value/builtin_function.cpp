#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::builtin_function::builtin_function(
    const std::function<base*(const std::vector<base*>&, environment&)>& body,
    environment& env)
  : base   {nullptr, env},
    m_body {body}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}

value::base* value::builtin_function::call(const std::vector<base*>& args)
{
  return m_body(args, *env().parent());
}

value::base* value::builtin_function::copy() const
{
  return gc::alloc<builtin_function>( m_body, *env().parent() );
}
