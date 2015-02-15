#include "integer.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace vv;

value::integer::integer(int val)
  : base {&builtin::type::integer},
    val  {val}
{ }

std::string value::integer::value() const { return std::to_string(val); }

size_t value::integer::hash() const
{
  const static std::hash<int> hasher{};
  return hasher(val);
}

bool value::integer::equals(const base& other) const
{
  if (other.type != &builtin::type::integer)
    return false;
  return static_cast<const integer&>(other).val == val;
}
