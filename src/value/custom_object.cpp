#include "custom_object.h"

#include "gc.h"
#include "ast/function_definition.h"
#include "value/custom_type.h"
#include "value/nil.h"

using namespace il;

value::custom_object::custom_object(custom_type* type,
                                    const std::vector<base*>& args,
                                    environment& outer_env)
  : m_local_env {outer_env},
    m_type      {type}
{
  const auto& mems = type->ctr_args();
  if (!type->ctr()) {
    if (args.size() != mems.size())
      throw std::runtime_error{"wrong number of arguments"};

    for (size_t i = args.size(); i--;)
      m_local_env.assign(mems[i], args[i]);
  } else {
    for (size_t i = mems.size(); i--;)
      m_local_env.assign(mems[i], gc::alloc<nil>( ));

    const auto fn = gc::push_argument(type->ctr()->eval(m_local_env));
    fn->call(args);
    gc::pop_argument();
  }
}

value::basic_type* value::custom_object::type() const
{
  return m_type;
}

std::string value::custom_object::value() const
{
  return "<object>";
}

value::base* value::custom_object::call_method(il::symbol method,
                                               const std::vector<base*>& args)
{
  const auto fn = gc::push_argument(type()->method(method, this, m_local_env));
  auto tmp = fn->call(args);
  gc::pop_argument();
  return tmp;
}

value::base* value::custom_object::copy() const
{
  throw std::runtime_error{"not yet implemented"};
}

void value::custom_object::mark()
{
  base::mark();
  m_type->mark();
  m_local_env.mark();
}
