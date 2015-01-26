#ifndef IL_VALUE_FUNCTION_H
#define IL_VALUE_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class function : public base {
public:
  function(const std::vector<symbol>& args,
           std::unique_ptr<ast::expression>&& body);
  base* type() const override;
  std::string value() const override;

private:
  std::vector<symbol> m_args;
  std::unique_ptr<ast::expression> m_body;
};

}

}

#endif
