#ifndef IL_VALUE_FLOATING_POINT_H
#define IL_VALUE_FLOATING_POINT_H

#include "value.h"

namespace il {

namespace value {

class floating_point : public base {
public:
  floating_point(double val, environment& env);

  double float_val() const { return m_val; }

  std::string value() const override;
  base* copy() const override;

private:
  double m_val;
};

}

}

#endif
