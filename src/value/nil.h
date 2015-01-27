#ifndef IL_VALUE_NIL_H
#define IL_VALUE_NIL_H

#include "value.h"

namespace il {

namespace value {

class nil : public base {
public:
  custom_type* type() const override;
  std::string value() const override;

private:
  static custom_type s_type;
};

}

}

#endif
