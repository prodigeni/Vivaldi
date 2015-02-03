#ifndef IL_VALUE_INTEGER_H
#define IL_VALUE_INTEGER_H

#include "value.h"

namespace il {

namespace value {

class integer : public base {
public:
  integer(int val, environment& env);

  int int_val() const { return m_val; }

  std::string value() const override;
  base* copy() const override;

private:
  int m_val;
};

}

}

#endif
