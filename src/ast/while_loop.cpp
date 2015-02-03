#include "while_loop.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/nil.h"

using namespace il;

ast::while_loop::while_loop(std::unique_ptr<expression>&& test,
                            std::unique_ptr<expression>&& body)
  : m_test {move(test)},
    m_body {move(body)}
{ }

value::base* ast::while_loop::eval(environment& env) const
{
  value::base* result{nullptr};
  environment inner_env{env};
  while (truthy(m_test->eval(inner_env))) {
    if (result)
      gc::pop_argument();
    result = gc::push_argument(m_body->eval(inner_env));
  }
  return result ? result : gc::alloc<value::nil>( env );
}
