#include "custom_type.h"

#include "gc.h"
#include "value/custom_object.h"

using namespace il;

value::custom_type::custom_type(const std::vector<il::symbol>& args,
           ast::expression* body,
           environment& outer_env)
  : m_env{outer_env}
{
  throw std::runtime_error{"not yet implemented"};
}

value::custom_type* value::custom_type::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::custom_type::value() const
{
  throw std::runtime_error{"not yet implemented"};
}

value::base* value::custom_type::call(const std::vector<base*>& args)
{
  return gc::alloc<custom_object>(this, m_ctr_args, m_ctr_body, m_env);
}

