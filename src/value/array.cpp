#include "array.h"

#include "gc.h"
#include "builtins.h"
#include "value/builtin_type.h"

using namespace il;

value::array::array(const std::vector<base*>& mems, environment& env)
  : base  {&builtin::type::array, env},
    m_mems {mems}
{ }

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
  return gc::alloc<array>( m_mems, *env().parent() );
}

void value::array::mark()
{
  base::mark();
  for (auto* i : m_mems)
    i->mark();
}
