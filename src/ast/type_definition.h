#ifndef IL_AST_TYPE_DEFINITION_H
#define IL_AST_TYPE_DEFINITION_H

#include "expression.h"

#include "symbol.h"
#include "ast/function_definition.h"

#include <unordered_map>

namespace il {

namespace ast {

class type_definition : public expression {
public:
  type_definition(symbol name,
                  symbol parent,
                  const std::unordered_map<
                            il::symbol,
                            std::shared_ptr<ast::function_definition>>&
                          m_methods);


  std::vector<vm::command> generate() const override;

private:
  symbol m_name;
  symbol m_parent;

  std::unordered_map<il::symbol,
                     std::shared_ptr<ast::function_definition>> m_methods;
};

}

}

#endif
