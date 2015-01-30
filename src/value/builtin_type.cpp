#include "builtin_type.h"

#include "gc.h"
#include "value/builtin_function.h"

using namespace il;

value::builtin_type::builtin_type(
    const std::function<base*(const std::vector<base*>&)>& ctr,
    const std::unordered_map<
            il::symbol,
            std::function<base*(value::base*, const std::vector<base*>&)>>& fns)
  : m_ctr      {ctr},
    m_methods  {fns}
{ }

value::basic_type* value::builtin_type::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::builtin_type::value() const
{
  return "<builtin type>";
}

value::base* value::builtin_type::method(il::symbol name,
                                         base* self,
                                         environment&) const
{
  using namespace std::placeholders;
  auto function = std::bind(m_methods.at(name), self, _1);
  return gc::alloc<builtin_function>( function );
}

value::base* value::builtin_type::call(const std::vector<base*>& args)
{
  return m_ctr(args);
}

value::base* value::builtin_type::copy() const
{
  return gc::alloc<value::builtin_type>( m_ctr, m_methods );
}
