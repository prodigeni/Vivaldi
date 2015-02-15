#ifndef VV_VALUE_FLOATING_POINT_H
#define VV_VALUE_FLOATING_POINT_H

#include "value.h"

namespace vv {

namespace value {

struct floating_point : public base {
public:
  floating_point(double val);

  std::string value() const override;
  size_t hash() const override;
  bool equals(const base& other) const override;

  double val;
};

}

}

#endif
