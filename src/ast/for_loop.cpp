#include "for_loop.h"

using namespace il;

ast::for_loop::for_loop(symbol iterator,
                        std::unique_ptr<expression>&& range,
                        std::unique_ptr<expression>&& body)
  : m_iterator {iterator},
    m_range    {move(range)},
    m_body     {move(body)}
{ }

value::base* ast::for_loop::eval(environment& env) const
{
  throw std::runtime_error{"not yet implemented"};
}
