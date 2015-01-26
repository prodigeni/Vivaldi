#include "function_definition.h"

using namespace il;

ast::function_definition::function_definition(symbol name,
                                              std::unique_ptr<expression>&&body,
                                              const std::vector<symbol>& args)
  : m_name {name},
    m_body {move(body)},
    m_args {args}
{ }

value::base* ast::function_definition::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
