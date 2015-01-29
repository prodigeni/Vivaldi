#include "custom_type.h"

#include "gc.h"
#include "value/custom_object.h"

using namespace il;

value::custom_type::custom_type(
    const std::vector<il::symbol>& args,
    const std::unordered_map<
              il::symbol,
              std::shared_ptr<ast::function_definition>>& methods,
    environment& outer_env)

  : m_ctr_args {args},
    m_env      {outer_env},
    m_methods  {methods}
{ }

value::custom_type* value::custom_type::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::custom_type::value() const
{
  return "<type>";
}

ast::function_definition* value::custom_type::method(il::symbol name) const
{
  return m_methods.at(name).get();
}

value::base* value::custom_type::call(const std::vector<base*>& args)
{
  return gc::alloc<custom_object>(this, args, m_env);
}

value::base* value::custom_type::copy() const
{
  return gc::alloc<value::custom_type>( m_ctr_args, m_methods, m_env );
}
