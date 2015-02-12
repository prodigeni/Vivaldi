#include "string_iterator.h"

#include "builtins.h"
#include "value/string.h"

using namespace vv;

value::string_iterator::string_iterator(string& new_str)
  : base {&builtin::type::string_iterator},
    str  {new_str},
    idx  {0}
{ }

std::string value::string_iterator::value() const
{
  return "<string iterator>";
}

void value::string_iterator::mark()
{
  base::mark();
  if (!str.marked())
    str.mark();
}
