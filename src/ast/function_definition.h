#ifndef IL_AST_FUNCTION_DEFINITION_H
#define IL_AST_FUNCTION_DEFINITION_H

#include "expression.h"

namespace il {

namespace ast {

class function_definition : public expression {
public:
  function_definition(symbol name,
                      std::unique_ptr<expression>&& body,
                      const std::vector<symbol>& args);

  value::base* eval(environment& env) const override;

private:
  symbol m_name;
  std::unique_ptr<expression> m_body;
  std::vector<symbol> m_args;
};

}

}

#endif
