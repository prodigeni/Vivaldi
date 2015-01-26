#include "nil.h"

#include <string>

using namespace il;

value::base* value::nil::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::nil::value() const { return "nil"; }
