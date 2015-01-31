#ifndef IL_VALUE_CUSTOM_OBJECT_H
#define IL_VALUE_CUSTOM_OBJECT_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class custom_object : public base {
public:
  custom_object(custom_type* type,
                const std::vector<base*>& args,
                environment& outer_env);

  basic_type* type() const override;
  std::string value() const override;

  base* call_method(il::symbol method, const std::vector<base*>& args) override;

  base* copy() const override;

  void mark() override;

private:

  environment m_local_env;

  custom_type* m_type;
};

}

}

#endif
