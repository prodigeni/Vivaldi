#ifndef VV_AST_array_H
#define VV_AST_array_H

#include "expression.h"

namespace vv {

namespace ast {

class array : public expression {
public:
  array(std::vector<std::unique_ptr<ast::expression>>&& members);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_function;
  std::vector<std::unique_ptr<ast::expression>> m_members;
};

}

}

#endif
