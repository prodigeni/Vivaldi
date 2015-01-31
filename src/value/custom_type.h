#ifndef IL_VALUE_CUSTOM_TYPE_H
#define IL_VALUE_CUSTOM_TYPE_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class custom_type : public basic_type {
public:
  custom_type(const std::vector<il::symbol>& args,
              const std::unordered_map<
                        il::symbol,
                        std::shared_ptr<ast::function_definition>>& methods,
             environment& outer_env);

  basic_type* type() const override;
  std::string value() const override;

  base* method(il::symbol name, base* self, environment& env) const override;
  const std::vector<il::symbol>& ctr_args() const { return m_ctr_args; }

  ast::function_definition* ctr() const { return m_ctr.get(); }

  base* call(const std::vector<base*>& args) override;

  base* copy() const override;

  void mark() override;

private:
  std::vector<il::symbol> m_ctr_args;
  environment& m_env;

  std::shared_ptr<ast::function_definition> m_ctr;

  std::unordered_map<il::symbol,
                     std::shared_ptr<ast::function_definition>> m_methods;
};

}

}

#endif
