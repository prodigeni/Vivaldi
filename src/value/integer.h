#ifndef VV_VALUE_INTEGER_H
#define VV_VALUE_INTEGER_H

#include "value.h"

namespace vv {

namespace value {

struct integer : public base {
public:
  integer(int val = 0);

  std::string value() const override;
  size_t hash() const override;
  bool equals(const base& other) const override;

  int val;
};

}

}

#endif
