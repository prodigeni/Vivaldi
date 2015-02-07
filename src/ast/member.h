#ifndef IL_AST_MEMBER_H
#define IL_AST_MEMBER_H

#include "expression.h"

namespace il {

namespace ast {

class member : public expression {
public:
  member(std::unique_ptr<ast::expression>&& object, il::symbol name);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_object;
  il::symbol m_name;
};

}

}

#endif
