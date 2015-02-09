#ifndef VV_AST_VARIABLE_H
#define VV_AST_VARIABLE_H

#include "expression.h"

#include "symbol.h"

namespace vv {

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
