#ifndef IL_VALUE_INTEGER_H
#define IL_VALUE_INTEGER_H

#include "value.h"

namespace il {

namespace value {

class integer : public base {
public:
  integer(int val) : m_val{val} { }

  basic_type* type() const override;
  std::string value() const override;

  int int_val() const { return m_val; }

  base* copy() const override;

private:
  int m_val;
};

}

}

#endif
