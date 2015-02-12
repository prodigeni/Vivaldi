#include "for_loop.h"

#include "vm/instruction.h"

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
  auto vec = m_range->generate();
  vec.emplace_back(vm::instruction::readm, symbol{"start"});
  vec.emplace_back(vm::instruction::call, 0);

  vec.emplace_back(vm::instruction::eblk);

  auto test_idx = vec.size();
  vec.emplace_back(vm::instruction::push);
  vec.emplace_back(vm::instruction::readm, symbol{"at_end"});
  vec.emplace_back(vm::instruction::call, 0);
  vec.emplace_back(vm::instruction::jmp_true);
  auto jmp_to_end_idx = vec.size() - 1;
  vec.emplace_back(vm::instruction::pop);
  vec.emplace_back(vm::instruction::push);
  vec.emplace_back(vm::instruction::readm, symbol{"get"});
  vec.emplace_back(vm::instruction::call, 0);
  vec.emplace_back(vm::instruction::let, m_iterator);

  auto body = m_body->generate();
  copy(begin(body), end(body), back_inserter(vec));

  vec.emplace_back(vm::instruction::pop);
  vec.emplace_back(vm::instruction::readm, symbol{"increment"});
  vec.emplace_back(vm::instruction::call, 0);
  vec.emplace_back(vm::instruction::jmp);
  auto vec_sz = static_cast<int>(vec.size() - 1);
  vec.back().arg = static_cast<int>(test_idx) - vec_sz;
  vec[jmp_to_end_idx].arg = static_cast<int>(vec.size() - jmp_to_end_idx);
  vec.emplace_back(vm::instruction::lblk);
  vec.emplace_back(vm::instruction::push_nil);

  return vec;
}
