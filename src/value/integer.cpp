#include "integer.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace il;

value::integer::integer(int val)
  : base {&builtin::type::integer},
    val  {val}
{ }

std::string value::integer::value() const { return std::to_string(val); }
