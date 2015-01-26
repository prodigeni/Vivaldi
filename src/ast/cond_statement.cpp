#include "cond_statement.h"

using namespace il;

ast::cond_statement::cond_statement(
    std::vector<std::pair<std::unique_ptr<expression>,
                std::unique_ptr<expression>>>&& body)
  : m_body {move(body)}
{ }

value::base* ast::cond_statement::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
