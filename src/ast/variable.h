#ifndef IL_AST_VARIABLE_H
#define IL_AST_VARIABLE_H

#include "expression.h"

namespace il {

namespace ast {

class variable : public expression {
public:
  variable(symbol name);

  value::base* eval(environment& env) const override;

private:
  symbol m_name;
};

}

}

#endif
