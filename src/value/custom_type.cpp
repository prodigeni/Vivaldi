#include "custom_type.h"

#include "gc.h"
#include "ast/function_definition.h"
#include "value/custom_object.h"

using namespace il;
using namespace value;

custom_type::custom_type(
    const std::vector<il::symbol>& args,
    const std::unordered_map<
              il::symbol,
              std::shared_ptr<ast::function_definition>>& methods,
    environment& outer_env)

  : basic_type {outer_env},
    m_ctr_args {args},
    m_methods  {methods}
{
  m_ctr = m_methods[{"init"}];
}

void custom_type::each_key(const std::function<void(il::symbol)>& fn) const
{
  for (const auto& i : m_methods)
    fn(i.first);
}

base* custom_type::method(il::symbol name, environment& env) const
{
  return m_methods.at(name)->eval(env);
}

std::string custom_type::value() const
{
  return "<type>";
}

base* value::custom_type::call(const std::vector<base*>& args)
{
  return gc::alloc<custom_object>(this, args, env());
}

base* value::custom_type::copy() const
{
  return gc::alloc<custom_type>(m_ctr_args, m_methods, *env().parent());
}

void custom_type::mark()
{
  base::mark();
}
