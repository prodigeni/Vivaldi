#ifndef VV_AST_OBJECT_CREATION_H
#define VV_AST_OBJECT_CREATION_H

#include "expression.h"

namespace vv {

namespace ast {

class object_creation : public expression {
public:
  object_creation(std::unique_ptr<ast::expression>&& type,
                  std::vector<std::unique_ptr<ast::expression>>&& args);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_type;
  std::vector<std::unique_ptr<ast::expression>> m_args;
};

}

}

#endif
