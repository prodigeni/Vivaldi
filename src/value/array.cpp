#include "array.h"

#include "gc.h"
#include "builtins.h"
#include "value/builtin_type.h"

using namespace il;

value::array::array(const std::vector<base*>& mems) : m_mems {mems} { }

value::basic_type* value::array::type() const
{
  return &builtin::type::array;
}

std::string value::array::value() const
{
  std::string str{'['};
  if (m_mems.size()) {
    for_each(begin(m_mems), end(m_mems) - 1,
             [&](const auto& v) { str += v->value() += ", "; });
    str += m_mems.back()->value();
  }
  str += ']';
  return str;
}

value::base* value::array::copy() const
{
  return gc::alloc<array>( m_mems );
}
