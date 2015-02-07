#ifndef IL_AST_LITERAL_H
#define IL_AST_LITERAL_H

#include "expression.h"

namespace il {

namespace ast {

class literal : public expression {
public:
  literal(std::unique_ptr<value::base>&& value);

  std::vector<vm::command> generate() const override;

  ~literal();

private:
  value::base* m_value;
};

}

}

#endif
