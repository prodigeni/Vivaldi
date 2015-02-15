#include "value/symbol.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::symbol::symbol(vv::symbol val)
  : base {&builtin::type::symbol},
    val  {val}
{ }

std::string value::symbol::value() const { return '\'' + to_string(val); }

size_t value::symbol::hash() const
{
  const static std::hash<vv::symbol> hasher{};
  return hasher(val);
}

bool value::symbol::equals(const base& other) const
{
  if (other.type != &builtin::type::symbol)
    return false;
  return static_cast<const symbol&>(other).val == val;
}
