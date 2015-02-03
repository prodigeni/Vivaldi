#include "nil.h"

#include "gc.h"
#include "builtins.h"
#include "value/builtin_type.h"

#include <string>

using namespace il;

value::nil::nil(environment& env)
  : base {&builtin::type::nil, env}
{ }

std::string value::nil::value() const { return "nil"; }

value::base* value::nil::copy() const
{
  return gc::alloc<nil>( *env().parent() );
}
