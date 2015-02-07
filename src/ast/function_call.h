#ifndef IL_AST_FUNCTION_CALL_H
#define IL_AST_FUNCTION_CALL_H

#include "expression.h"

namespace il {

namespace ast {

class function_call : public expression {
public:
  function_call(std::unique_ptr<ast::expression>&& name,
                std::vector<std::unique_ptr<ast::expression>>&& args);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_function;
  std::vector<std::unique_ptr<ast::expression>> m_args;
};

}

}

#endif
