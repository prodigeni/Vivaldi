#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_type.h"

using namespace il;

value::builtin_function::builtin_function(
    const std::function<base*(const std::vector<base*>&)>& body)
  : base   {nullptr},
    m_body {body}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}
