#include "method_call.h"

#include "gc.h"

using namespace il;

ast::method_call::method_call(
    std::unique_ptr<ast::expression>&& object,
    il::symbol name,
    std::vector<std::unique_ptr<ast::expression>>&& args)
  : m_object {move(object)},
    m_name   {name},
    m_args   {move(args)}
{ }

value::base* ast::method_call::eval(environment& env) const
{
  auto object = m_object->eval(env);

  std::vector<value::base*> args;
  std::transform(begin(m_args), end(m_args), back_inserter(args),
                 [&](const auto& i){ return gc::push_argument(i->eval(env)); });
  auto result = object->call_method(m_name, args);
  for (auto i = m_args.size(); i--;)
    gc::pop_argument();
  return result;
}
