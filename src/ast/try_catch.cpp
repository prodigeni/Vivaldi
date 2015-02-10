#include "try_catch.h"

#include "gc.h"

using namespace vv;

ast::try_catch::try_catch(std::unique_ptr<expression>&& body,
                          std::unique_ptr<expression>&& catcher)
  : m_body    {move(body)},
    m_catcher {move(catcher)}
{ }

std::vector<vm::command> ast::try_catch::generate() const
{
  auto vec = m_catcher->generate();
  vec.emplace_back(vm::instruction::push_catch);

  auto body = m_body->generate();
  body.emplace_back(vm::instruction::ret);
  vec.emplace_back(vm::instruction::push_fn, move(body));
  vec.emplace_back(vm::instruction::call, 0);

  vec.emplace_back(vm::instruction::pop_catch);
  return vec;
}
