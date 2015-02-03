#ifndef IL_VALUE_BUILTIN_FUNCTION_H
#define IL_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class builtin_function : public base {
public:
  builtin_function(
      const std::function<base*(const std::vector<base*>&, environment&)>& body,
      environment& env);

  std::string value() const override;
  base* call(const std::vector<base*>& args) override;
  base* copy() const override;

private:
  std::function<base*(const std::vector<base*>&, environment&)> m_body;
};

}

}

#endif
