#include "range.h"

#include "builtins.h"

using namespace vv;

value::range::range(value::base& new_start, value::base& new_end)
  : base  {&builtin::type::range},
    start {&new_start},
    end   {new_end}
{ }

std::string value::range::value() const
{
  return start->value() + " to " + end.value();
}

void value::range::mark()
{
  base::mark();
  if (!start->marked())
    start->mark();
  if (!end.marked())
    end.mark();
}
