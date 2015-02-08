#include "custom_object.h"

#include "gc.h"
#include "ast/function_definition.h"
#include "value/nil.h"

using namespace il;

value::custom_object::custom_object(value::type* type,
                                    const std::vector<base*>& args)
  : base {type}
{ }

std::string value::custom_object::value() const
{
  return "<object>";
}
