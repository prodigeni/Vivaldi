#include "lang_utils.h"

#include "builtins.h"
#include "gc.h"
#include "value/boolean.h"

bool il::truthy(const value::base* val)
{
  if (val->type == &builtin::type::nil)
    return false;
  else if (val->type == &builtin::type::boolean)
    return static_cast<const value::boolean*>(val)->val;
  return true;
}

void il::check_size(size_t expected, size_t receieved)
{
  if (expected != receieved)
    throw std::runtime_error{"wrong number of arguments (expected "    +
                             std::to_string(expected)  += ", got " +
                             std::to_string(receieved) += ")"};
}
