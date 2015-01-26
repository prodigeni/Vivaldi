#include "boolean.h"

#include <string>

using namespace il;

value::base* value::boolean::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::boolean::value() const
{
  return m_val ? "true" : "false";
}
