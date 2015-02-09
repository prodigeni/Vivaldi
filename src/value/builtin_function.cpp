#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::builtin_function::builtin_function(
    const std::function<base*(vm::call_stack&)>& new_body)
  : base {nullptr},
    body {new_body}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}
