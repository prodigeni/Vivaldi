#ifndef VV_AST_RETURN_STATEMENT_H
#define VV_AST_RETURN_STATEMENT_H

#include "expression.h"

namespace vv {

namespace ast {

class return_statement : public expression {
public:
  return_statement(std::unique_ptr<expression>&& value);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_value;
};

}

}

#endif
