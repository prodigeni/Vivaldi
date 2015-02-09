#ifndef VV_AST_WHILE_LOOP_H
#define VV_AST_WHILE_LOOP_H

#include "expression.h"

namespace vv {

namespace ast {

class while_loop : public expression {
public:
  while_loop(std::unique_ptr<expression>&& test,
             std::unique_ptr<expression>&& body);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_test;
  std::unique_ptr<expression> m_body;
};

}

}

#endif
