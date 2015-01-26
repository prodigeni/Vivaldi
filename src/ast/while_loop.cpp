#include "while_loop.h"

using namespace il;

ast::while_loop::while_loop(std::unique_ptr<expression>&& test,
                            std::unique_ptr<expression>&& body)
  : m_test {move(test)},
    m_body {move(body)}
{ }

value::base* ast::while_loop::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
