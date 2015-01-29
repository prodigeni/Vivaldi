#ifndef IL_AST_TYPE_DEFINITION_H
#define IL_AST_TYPE_DEFINITION_H

#include "expression.h"

namespace il {

namespace ast {

class type_definition : public expression {
public:
  type_definition(symbol name,
                  symbol parent,
                  const std::vector<symbol>& public_mems,
                  const std::unordered_map<
                            il::symbol,
                            std::shared_ptr<ast::function_definition>>&
                          m_methods);


  value::base* eval(environment& env) const override;

private:
  symbol m_name;
  symbol m_parent;

  std::vector<symbol> m_members;

  std::unordered_map<il::symbol,
                     std::shared_ptr<ast::function_definition>> m_methods;
};

}

}

#endif
