#include "lang_utils.h"

bool il::truthy(const value::base* val)
{
  throw std::runtime_error{"not yet implemented"};
}

void il::check_size(size_t expected, size_t receieved)
{
  if (expected != receieved)
    throw std::runtime_error{"wrong number of arguments (expected "    +
                             std::to_string(expected)  += ", got " +
                             std::to_string(receieved) += ")"};
}
