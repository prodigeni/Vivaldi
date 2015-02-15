#include "boolean.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"

#include <string>

using namespace vv;

value::boolean::boolean(bool new_val)
  : base {&builtin::type::boolean},
    val  {new_val}
{ }

std::string value::boolean::value() const { return val ? "true" : "false"; }

size_t value::boolean::hash() const
{
  // This seems almost metaphysical in its overkill. How much entropy am I
  // expecting to get out of a single bit? Still, no reason *not* to stick with
  // the pattern, I guess.
  const static std::hash<bool> hasher{};
  return hasher(val);
}

bool value::boolean::equals(const base& other) const
{
  if (other.type != &builtin::type::boolean)
    return false;
  return static_cast<const boolean&>(other).val == val;
}
