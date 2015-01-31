#include "function_call.h"

#include "gc.h"

using namespace il;

ast::function_call::function_call(
    std::unique_ptr<ast::expression>&& name,
    std::vector<std::unique_ptr<ast::expression>>&& args)
  : m_function_name {move(name)},
    m_args          {move(args)}
{ }

value::base* ast::function_call::eval(environment& env) const
{
  auto fn = m_function_name->eval(env);

  std::vector<value::base*> args;
  std::transform(begin(m_args), end(m_args), back_inserter(args),
                 [&](const auto& i){ return gc::push_argument(i->eval(env)); });
  auto tmp = fn->call(args);
  for (auto i = m_args.size(); i--;)
    gc::pop_argument();
  return tmp;
}
