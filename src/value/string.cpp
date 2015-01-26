#include "value/string.h"

using namespace il;

value::base* value::string::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::string::value() const
{
  return '"' + m_val += '"';
}
