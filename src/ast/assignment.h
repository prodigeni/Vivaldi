#ifndef IL_AST_ASSIGNMENT_H
#define IL_AST_ASSIGNMENT_H

#include "expression.h"

namespace il {

namespace ast {

class assignment : public expression {
public:
  assignment(symbol name, std::unique_ptr<expression>&& value);

  std::vector<vm::command> generate() const override;

private:
  symbol m_name;
  std::unique_ptr<expression> m_value;
};

}

}

#endif
