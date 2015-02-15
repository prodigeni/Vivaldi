#include "value/string.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::string::string(const std::string& val)
  : base {&builtin::type::string},
    val  {val}
{ }

std::string value::string::value() const { return '"' + val += '"'; }

size_t value::string::hash() const
{
  const static std::hash<std::string> hasher{};
  return hasher(val);
}

bool value::string::equals(const base& other) const
{
  if (other.type != &builtin::type::string)
    return false;
  return static_cast<const string&>(other).val == val;
}
