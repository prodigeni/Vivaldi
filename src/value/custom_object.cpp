#include "custom_object.h"

#include "gc.h"
#include "ast/function_definition.h"
#include "value/custom_type.h"
#include "value/nil.h"

using namespace il;

value::custom_object::custom_object(custom_type* type,
                                    const std::vector<base*>& args,
                                    environment& outer_env)
  : base {type, outer_env}
{
  const auto& mems = type->ctr_args();
  for (size_t i = mems.size(); i--;)
    env().assign(mems[i], gc::alloc<nil>( env() ));

  const auto fn = gc::push_argument(type->ctr()->eval(env()));
  fn->call(args);
  gc::pop_argument();
}

std::string value::custom_object::value() const
{
  return "<object>";
}

value::base* value::custom_object::copy() const
{
  throw std::runtime_error{"not yet implemented"};
}

void value::custom_object::mark()
{
  base::mark();
}
