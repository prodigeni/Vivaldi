#include "floating_point.h"

#include <string>

using namespace il;

value::base* value::floating_point::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::floating_point::value() const
{
  return std::to_string(m_val);
}
