#include "integer.h"

#include <string>

using namespace il;

value::custom_type* value::integer::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::integer::value() const { return std::to_string(m_val); }
