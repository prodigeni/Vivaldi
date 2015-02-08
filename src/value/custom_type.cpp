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
              std::shared_ptr<ast::function_definition>>& methods)

  : basic_type {},
    m_ctr_args {args},
    m_methods  {methods}
{
  m_ctr = m_methods[{"init"}];
}

std::string custom_type::value() const
{
  return "<type>";
}
