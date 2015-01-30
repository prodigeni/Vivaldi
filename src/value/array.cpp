#include "array.h"

#include "gc.h"

using namespace il;

value::array::array(const std::vector<base*>& mems) : m_mems {mems} { }

value::custom_type* value::array::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::array::value() const
{
  std::string str{'['};
  for_each(begin(m_mems), end(m_mems) - 1,
           [&](const auto& v) { str += v->value() += ", "; });
  if (m_mems.size())
    str += m_mems.back()->value() += ']';
  return str;
}

value::base* value::array::copy() const
{
  return gc::alloc<array>( m_mems );
}
