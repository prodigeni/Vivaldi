#include "array.h"

#include "gc.h"
#include "builtins.h"

using namespace vv;

value::array::array(const std::vector<base*>& new_val)
  : base {&builtin::type::array},
    val {new_val}
{ }

std::string value::array::value() const
{
  std::string str{'['};
  if (val.size()) {
    for_each(begin(val), end(val) - 1,
             [&](const auto& v) { str += v->value() += ", "; });
    str += val.back()->value();
  }
  str += ']';
  return str;
}

void value::array::mark()
{
  base::mark();
  for (auto* i : val)
    if (!i->marked())
      i->mark();
}
