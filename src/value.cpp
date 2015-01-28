#include "value.h"

using namespace il;

value::base* value::base::call(const std::vector<value::base*>& args)
{
  throw std::runtime_error{"not yet implemented"};
}
