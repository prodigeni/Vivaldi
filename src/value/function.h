#ifndef IL_VALUE_FUNCTION_H
#define IL_VALUE_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class function : public base {
public:
  function(const std::vector<il::symbol>& args,
           std::shared_ptr<ast::expression> body,
           environment& outer_env);

  basic_type* type() const override;
  std::string value() const override;

  base* call(const std::vector<base*>& args) override;

  base* copy() const override;

private:
  std::vector<il::symbol> m_args;
  std::shared_ptr<ast::expression> m_body;
  environment& m_env;
};

}

}

#endif
