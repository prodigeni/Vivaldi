#include "floating_point.h"

#include "builtins.h"
#include "gc.h"

#include <string>

using namespace il;

value::floating_point::floating_point(double value)
  : base {&builtin::type::floating_point},
    val  {value}
{ }

std::string value::floating_point::value() const
{
  return std::to_string(val);
}
