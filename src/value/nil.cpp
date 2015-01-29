#include "nil.h"

#include "gc.h"

#include <string>

using namespace il;

value::custom_type* value::nil::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::nil::value() const { return "nil"; }

value::base* value::nil::copy() const
{
  return gc::alloc<nil>();
}
