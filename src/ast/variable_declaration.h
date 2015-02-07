#ifndef IL_AST_VARIABLE_DECLARATION_H
#define IL_AST_VARIABLE_DECLARATION_H

#include "expression.h"

#include "symbol.h"

namespace il {

namespace ast {

class variable_declaration : public expression {
public:
  variable_declaration(symbol name, std::unique_ptr<expression>&& value);

  std::vector<vm::command> generate() const override;

private:
  symbol m_name;
  std::unique_ptr<expression> m_value;
};

}

}

#endif
