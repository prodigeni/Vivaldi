#ifndef IL_VALUE_CUSTOM_TYPE_H
#define IL_VALUE_CUSTOM_TYPE_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class custom_type : public base {
public:
  custom_type(const std::vector<il::symbol>& args,
             ast::expression* body,
             environment& outer_env);

  custom_type* type() const override;
  std::string value() const override;

  base* call(const std::vector<base*>& args) override;

  base* copy() const override;

private:
  std::vector<il::symbol> m_ctr_args;
  ast::expression* m_ctr_body;
  environment& m_env;

  std::vector<value::function*> m_methods;
  std::vector<value::function*> m_static_methods;
};

}

}

#endif
