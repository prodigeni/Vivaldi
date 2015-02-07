#ifndef IL_AST_COND_STATEMENT_H
#define IL_AST_COND_STATEMENT_H

#include "expression.h"

namespace il {

namespace ast {

class cond_statement : public expression {
public:
  cond_statement(std::vector<std::pair<std::unique_ptr<expression>,
                                       std::unique_ptr<expression>>>&& body);

  std::vector<vm::command> generate() const override;

private:
  std::vector<std::pair<std::unique_ptr<expression>,
                        std::unique_ptr<expression>>> m_body;
};

}

}

#endif
