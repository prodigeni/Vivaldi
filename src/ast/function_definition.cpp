#include "function_definition.h"

#include "gc.h"
#include "value/function.h"

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
  const static symbol nonname{""};
  if (m_name == nonname)
    return gc::alloc<value::function>(m_args, m_body, env);
  else
    return env.create(m_name, gc::alloc<value::function>(m_args, m_body, env));
}
