#ifndef IL_AST_ASSIGNMENT_H
#define IL_AST_ASSIGNMENT_H

#include "expression.h"

namespace il {

namespace ast {

class assignment : public expression {
public:
  assignment(symbol name, std::unique_ptr<expression>&& value);

  value::base* eval(environment& env) const override;

private:
  symbol m_name;
  std::unique_ptr<expression> m_value;
};

}

}

#endif
