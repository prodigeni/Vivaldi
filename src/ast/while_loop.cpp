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

std::vector<vm::command> ast::while_loop::generate() const
{
  auto vec = m_test->generate();
  vec.emplace_back(vm::instruction::jump_unless);
  auto test_jump_iterator = --end(vec);

  auto body = m_body->generate();
  copy(begin(body), end(body), back_inserter(vec));

  vec.emplace_back(vm::instruction::jump, -static_cast<int>(vec.size()));
  test_jump_iterator->arg = static_cast<int>(end(vec) - test_jump_iterator);

  return vec;
}
