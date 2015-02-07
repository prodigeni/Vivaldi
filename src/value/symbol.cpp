#include "value/symbol.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::symbol::symbol(il::symbol val)
  : base {&builtin::type::symbol},
    val  {val}
{ }

std::string value::symbol::value() const { return '\'' + to_string(val); }
