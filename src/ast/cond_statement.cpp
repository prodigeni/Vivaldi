#include "cond_statement.h"

#include "lang_utils.h"
#include "value/nil.h"
#include "vm/instruction.h"

using namespace vv;

ast::cond_statement::cond_statement(
    std::vector<std::pair<std::unique_ptr<expression>,
                std::unique_ptr<expression>>>&& body)
  : m_body {move(body)}
{ }

std::vector<vm::command> ast::cond_statement::generate() const
{
  std::vector<vm::command> vec;
  std::vector<size_t> jump_to_end_idxs;

  for (const auto& i : m_body) {
    auto test = i.first->generate();
    copy(begin(test), end(test), back_inserter(vec));
    vec.emplace_back(vm::instruction::jmp_false);
    auto jump_to_next_test_idx = vec.size() - 1;

    auto body = i.second->generate();
    copy(begin(body), end(body), back_inserter(vec));
    vec.emplace_back(vm::instruction::jmp);
    jump_to_end_idxs.push_back(vec.size() - 1);

    auto jump_sz = static_cast<int>(vec.size() - jump_to_next_test_idx);
    vec[jump_to_next_test_idx].arg = jump_sz;
  }

  vec.emplace_back(vm::instruction::push_nil);
  for (auto i : jump_to_end_idxs) {
    auto jump_sz = static_cast<int>(vec.size() - i);
    vec[i].arg = jump_sz;
  }

  return vec;
}
