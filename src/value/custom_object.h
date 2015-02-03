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

  std::string value() const override;
  base* copy() const override;

  void mark() override;
};

}

}

#endif
