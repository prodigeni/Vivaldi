#ifndef IL_AST_METHOD_CALL_H
#define IL_AST_METHOD_CALL_H

#include "expression.h"

namespace il {

namespace ast {

class method_call : public expression {
public:
  method_call(std::unique_ptr<ast::expression>&& object,
              il::symbol name,
              std::vector<std::unique_ptr<ast::expression>>&& args);

  value::base* eval(environment& env) const override;

private:
  std::unique_ptr<ast::expression> m_object;
  il::symbol m_name;
  std::vector<std::unique_ptr<ast::expression>> m_args;
};

}

}

#endif
