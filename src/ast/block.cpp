#include "block.h"

#include "gc.h"

using namespace il;

ast::block::block(std::vector<std::unique_ptr<expression>>&& subexpressions)
  : m_subexpressions {move(subexpressions)}
{ }

value::base* ast::block::eval(environment& env) const
{
  environment block_env{env};
  value::base* result{nullptr};
  for (const auto& i : m_subexpressions) {
    if (result)
      gc::pop_argument();
    result = gc::push_argument(i->eval(block_env));
  }
  return result;
}
