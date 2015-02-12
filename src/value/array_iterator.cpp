#include "array_iterator.h"

#include "builtins.h"
#include "value/array.h"

using namespace vv;

value::array_iterator::array_iterator(array& new_arr)
  : base {&builtin::type::array_iterator},
    arr  {new_arr},
    idx  {0}
{ }

std::string value::array_iterator::value() const { return "<array iterator>"; }

void value::array_iterator::mark()
{
  base::mark();
  if (!arr.marked())
    arr.mark();
}
