#ifndef IL_VALUE_CUSTOM_TYPE_H
#define IL_VALUE_CUSTOM_TYPE_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

struct custom_type : public basic_type {
public:
  custom_type(const std::vector<il::symbol>& args,
              const std::unordered_map<
                        il::symbol,
                        std::shared_ptr<ast::function_definition>>& methods);

  std::string value() const override;

  const std::vector<il::symbol>& ctr_args() const { return m_ctr_args; }
  ast::function_definition* ctr() const { return m_ctr.get(); }

private:
  std::vector<il::symbol> m_ctr_args;

  std::shared_ptr<ast::function_definition> m_ctr;
  std::unordered_map<il::symbol,
                     std::shared_ptr<ast::function_definition>> m_methods;
};

}

}

#endif
