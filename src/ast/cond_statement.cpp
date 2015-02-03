#include "cond_statement.h"

#include "lang_utils.h"
#include "gc.h"
#include "value/nil.h"

using namespace il;

ast::cond_statement::cond_statement(
    std::vector<std::pair<std::unique_ptr<expression>,
                std::unique_ptr<expression>>>&& body)
  : m_body {move(body)}
{ }

value::base* ast::cond_statement::eval(environment& env) const
{
  for (const auto& i : m_body) {
    environment pair_env{env};
    if (truthy(i.first->eval(pair_env)))
      return i.second->eval(pair_env);
  }
  return gc::alloc<value::nil>( env );
}
