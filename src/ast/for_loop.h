#ifndef VV_AST_FOR_LOOP_H
#define VV_AST_FOR_LOOP_H

#include "expression.h"

namespace vv {

namespace ast {

class for_loop : public expression {
public:
  for_loop(symbol iterator,
           std::unique_ptr<expression>&& range,
           std::unique_ptr<expression>&& body);

  std::vector<vm::command> generate() const override;

private:
  symbol m_iterator;
  std::unique_ptr<expression> m_range;
  std::unique_ptr<expression> m_body;
};

}

}

#endif
