#include "array.h"

#include "gc.h"
#include "builtins.h"

using namespace il;

value::array::array(const std::vector<base*>& new_mems)
  : base {&builtin::type::array},
    mems {new_mems}
{ }

std::string value::array::value() const
{
  std::string str{'['};
  if (mems.size()) {
    for_each(begin(mems), end(mems) - 1,
             [&](const auto& v) { str += v->value() += ", "; });
    str += mems.back()->value();
  }
  str += ']';
  return str;
}
