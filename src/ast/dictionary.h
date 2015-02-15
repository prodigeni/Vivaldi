#ifndef VV_AST_DICTIONARY_H
#define VV_AST_DICTIONARY_H

#include "expression.h"

namespace vv {

namespace ast {

class dictionary : public expression {
public:
  dictionary(std::vector<std::unique_ptr<ast::expression>>&& members);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_function;
  std::vector<std::unique_ptr<ast::expression>> m_members;
};

}

}

#endif
