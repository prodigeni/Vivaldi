#include "builtin_type.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_function.h"

using namespace il;
using namespace value;

namespace {

auto method_for(const std::function<base*(base*,
                                          const std::vector<base*>&)>& fn)
{
  return [&](const std::vector<base*>& args, environment& env)
  {
    auto self = env.at(builtin::sym::self);
    return fn(self, args);
  };
}

}

builtin_type::builtin_type(
    const std::function<base*(const std::vector<base*>&)>& ctr,
    const std::unordered_map<
            il::symbol,
            std::function<base*(base*, const std::vector<base*>&)>>& fns,
    environment& env)
  : basic_type {env},
    m_ctr      {ctr},
    m_methods  {fns}
{ }

void builtin_type::each_key(
    const std::function<void(il::symbol)>& fn) const
{
  for (const auto& i : m_methods)
    fn(i.first);
}

base* builtin_type::method(il::symbol name, environment& env) const
{
  return gc::alloc<builtin_function>( method_for(m_methods.at(name)), env );
}

std::string builtin_type::value() const
{
  return "<builtin type>";
}

base* builtin_type::call(const std::vector<base*>& args)
{
  return m_ctr(args);
}

base* builtin_type::copy() const
{
  return gc::alloc<builtin_type>( m_ctr, m_methods, *env().parent() );
}
