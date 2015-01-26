#ifndef IL_AST_LITERAL_H
#define IL_AST_LITERAL_H

#include "expression.h"

namespace il {

namespace ast {

class literal : public expression {
public:
  literal(std::unique_ptr<value::base>&& value);

  value::base* eval(environment& env) const override;

private:
  std::unique_ptr<value::base> m_value;
};

}

}

#endif
