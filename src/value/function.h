#ifndef IL_VALUE_FUNCTION_H
#define IL_VALUE_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class function : public base {
public:
  function(const std::vector<il::symbol>& args,
           ast::expression* body,
           environment& outer_env);

  custom_type* type() const override;
  std::string value() const override;

  base* call(const std::vector<base*>& args) override;

private:
  std::vector<il::symbol> m_args;
  ast::expression* m_body;
  environment& m_env;
};

}

}

#endif
