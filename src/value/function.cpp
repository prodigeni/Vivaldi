#include "function.h"

using namespace il;

value::function::function(const std::vector<il::symbol>& args,
                          ast::expression* body,
                          environment& outer_env)
  : m_args {args},
    m_body {body},
    m_env  {outer_env}
{ }

value::base* value::function::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::function::value() const
{
  return "<function>";
}

value::base* value::function::call(const std::vector<base*>& args)
{
  if (args.size() != m_args.size())
    throw std::runtime_error{"wrong number of arguments"};
  environment call_env{m_env};
  for (auto sz = args.size(); --sz;)
    call_env.assign(m_args[sz], args[sz]);

  return m_body->eval(call_env);
}
