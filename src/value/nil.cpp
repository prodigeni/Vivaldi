#include "nil.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace il;

value::nil::nil()
  : base {&builtin::type::nil}
{ }

std::string value::nil::value() const { return "nil"; }
