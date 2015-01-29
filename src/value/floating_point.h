#ifndef IL_VALUE_FLOATING_POINT_H
#define IL_VALUE_FLOATING_POINT_H

#include "value.h"

namespace il {

namespace value {

class floating_point : public base {
public:
  floating_point(double val) : m_val{val} { }

  custom_type* type() const override;
  std::string value() const override;

  base* copy() const override;

private:
  double m_val;
};

}

}

#endif
