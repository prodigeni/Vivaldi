#ifndef IL_VALUE_CUSTOM_TYPE_H
#define IL_VALUE_CUSTOM_TYPE_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class custom_type : public base {
public:
  custom_type(const std::vector<il::symbol>& args,
              const std::unordered_map<
                        il::symbol,
                        std::shared_ptr<ast::function_definition>>& methods,
             environment& outer_env);

  custom_type* type() const override;
  std::string value() const override;

  ast::function_definition* method(il::symbol name) const;
  const std::vector<il::symbol>& ctr_args() const { return m_ctr_args; }

  base* call(const std::vector<base*>& args) override;

  base* copy() const override;

private:
  std::vector<il::symbol> m_ctr_args;
  environment& m_env;

  std::unordered_map<il::symbol,
                     std::shared_ptr<ast::function_definition>> m_methods;
};

}

}

#endif
