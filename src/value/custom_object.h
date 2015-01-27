#ifndef IL_VALUE_CUSTOM_OBJECT_H
#define IL_VALUE_CUSTOM_OBJECT_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class custom_object : public base {
public:
  custom_object(const std::vector<il::symbol>& args,
                ast::expression* body,
                environment& outer_env);

  custom_type* type() const override;
  std::string value() const override;

  base* call(const std::vector<base*>& args) override;

private:

  std::vector<base*> m_members;
  custom_type* m_type;
};

}

}

#endif
