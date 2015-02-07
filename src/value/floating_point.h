#ifndef IL_VALUE_FLOATING_POINT_H
#define IL_VALUE_FLOATING_POINT_H

#include "value.h"

namespace il {

namespace value {

struct floating_point : public base {
public:
  floating_point(double val);

  std::string value() const override;

  double val;
};

}

}

#endif
