#include "floating_point.h"

#include "builtins.h"
#include "gc.h"

#include <string>

using namespace vv;

value::floating_point::floating_point(double value)
  : base {&builtin::type::floating_point},
    val  {value}
{ }

std::string value::floating_point::value() const
{
  return std::to_string(val);
}

size_t value::floating_point::hash() const
{
  const static std::hash<double> hasher{};
  return hasher(val);
}

bool value::floating_point::equals(const base& other) const
{
  if (other.type != &builtin::type::floating_point)
    return false;
  return static_cast<const floating_point&>(other).val == val;
}
