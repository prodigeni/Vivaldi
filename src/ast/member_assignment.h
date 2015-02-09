#ifndef IL_AST_MEMBER_ASSIGNMENT_H
#define IL_AST_MEMBER_ASSIGNMENT_H

#include "expression.h"

namespace il {

namespace ast {

class member_assignment : public expression {
public:
  member_assignment(std::unique_ptr<ast::expression>&& object,
                    il::symbol name,
                    std::unique_ptr<ast::expression>&& value);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_object;
  il::symbol m_name;
  std::unique_ptr<ast::expression> m_value;
};

}

}

#endif
