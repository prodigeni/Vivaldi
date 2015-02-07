#include "custom_object.h"

#include "gc.h"
#include "ast/function_definition.h"
#include "value/custom_type.h"
#include "value/nil.h"

using namespace il;

value::custom_object::custom_object(custom_type* type,
                                    const std::vector<base*>& args)
  : base {type}
{
  const auto fn = gc::push_argument(type->ctr()->eval());
  fn->call(args);
  gc::pop_argument();
}

std::string value::custom_object::value() const
{
  return "<object>";
}
