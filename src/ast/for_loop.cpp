#include "for_loop.h"

using namespace vv;

ast::for_loop::for_loop(symbol iterator,
                        std::unique_ptr<expression>&& range,
                        std::unique_ptr<expression>&& body)
  : m_iterator {iterator},
    m_range    {move(range)},
    m_body     {move(body)}
{ }

std::vector<vm::command> ast::for_loop::generate() const
{
  throw std::runtime_error{"not yet implemented"};
}
