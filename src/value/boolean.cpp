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
