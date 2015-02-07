#ifndef IL_VALUE_BUILTIN_FUNCTION_H
#define IL_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

struct builtin_function : public base {
public:
  builtin_function(const std::function<base*(const std::vector<base*>&)>& body);

  std::string value() const override;

private:
  std::function<base*(const std::vector<base*>&)> m_body;
};

}

}

#endif
