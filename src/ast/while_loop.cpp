#include "while_loop.h"

#include "lang_utils.h"
#include "value/nil.h"
#include "vm/instruction.h"

using namespace vv;

ast::while_loop::while_loop(std::unique_ptr<expression>&& test,
                            std::unique_ptr<expression>&& body)
  : m_test {move(test)},
    m_body {move(body)}
{ }

std::vector<vm::command> ast::while_loop::generate() const
{
  auto vec = m_test->generate();
  vec.emplace_back(vm::instruction::jmp_false);
  auto test_jump_idx = vec.size() - 1;

  auto body = m_body->generate();
  copy(begin(body), end(body), back_inserter(vec));

  vec.emplace_back(vm::instruction::jmp, -static_cast<int>(vec.size()));
  vec[test_jump_idx].arg = static_cast<int>(vec.size() - test_jump_idx);

  return vec;
}
