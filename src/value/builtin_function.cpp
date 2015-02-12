#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::builtin_function::builtin_function(
    const std::function<base*(vm::machine&)>& new_body,
    int new_argc)
  : base {&builtin::type::function},
    body {new_body},
    argc {new_argc}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}
