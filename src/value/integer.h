#ifndef VV_VALUE_INTEGER_H
#define VV_VALUE_INTEGER_H

#include "value.h"

namespace vv {

namespace value {

struct integer : public base {
public:
  integer(int val);

  std::string value() const override;

  int val;
};

}

}

#endif
