#include "builtin_type.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_function.h"

using namespace il;
using namespace value;

namespace {

}

builtin_type::builtin_type(
    const std::function<base*(vm::call_stack&)>& ctr,
    const std::unordered_map<
            il::symbol,
            value::base*>& fns)
  : basic_type {},
    m_ctr      {ctr}
{
  methods = fns;
}

std::string builtin_type::value() const
{
  return "<builtin type>";
}
