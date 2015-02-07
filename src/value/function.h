#ifndef IL_VALUE_FUNCTION_H
#define IL_VALUE_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

struct function : public base {
public:
  function(const std::vector<il::symbol>& args,
           std::shared_ptr<ast::expression> body);

  std::string value() const override;

private:
  std::vector<il::symbol> m_args;
  std::shared_ptr<ast::expression> m_body;
};

}

}

#endif
