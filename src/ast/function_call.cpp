#include "function_call.h"

using namespace il;

ast::function_call::function_call(
    symbol name,
    std::vector<std::unique_ptr<ast::expression>>&& args)
  : m_function_name {name},
    m_args          {move(args)}
{ }

value::base* ast::function_call::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
