#ifndef IL_AST_TYPE_DEFINITION_H
#define IL_AST_TYPE_DEFINITION_H

#include "expression.h"

namespace il {

namespace ast {

class type_definition : public expression {
public:
  type_definition(symbol name,
                  symbol parent,
                  const std::vector<std::unique_ptr<expression>> public_mems,
                  const std::vector<std::unique_ptr<expression>> private_mems);

  value::base* eval(environment& env) const override;

private:
  symbol m_name;
  symbol m_parent;
  std::vector<std::unique_ptr<expression>> m_public_members;
  std::vector<std::unique_ptr<expression>> m_private_members;
};

}

}

#endif
