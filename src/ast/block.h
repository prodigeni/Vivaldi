#ifndef IL_AST_BLOCK_H
#define IL_AST_BLOCK_H

#include "expression.h"

namespace il {

namespace ast {

class block : public expression {
public:
  block(std::vector<std::unique_ptr<expression>>&& subexpressions);

  std::vector<vm::command> generate() const override;

private:
  std::vector<std::unique_ptr<expression>> m_subexpressions;
};

}

}

#endif
