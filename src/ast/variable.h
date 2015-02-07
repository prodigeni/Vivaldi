#ifndef IL_AST_VARIABLE_H
#define IL_AST_VARIABLE_H

#include "expression.h"

#include "symbol.h"

namespace il {

namespace ast {

class variable : public expression {
public:
  variable(symbol name);

  std::vector<vm::command> generate() const override;

private:
  symbol m_name;
};

}

}

#endif
