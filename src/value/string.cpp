#include "value/string.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::string::string(const std::string& val)
  : base {&builtin::type::string},
    val  {val}
{ }

std::string value::string::value() const { return '"' + val += '"'; }
