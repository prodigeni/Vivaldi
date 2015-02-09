#ifndef VV_VALUE_NIL_H
#define VV_VALUE_NIL_H

#include "value.h"

namespace vv {

namespace value {

struct nil : public base {
  nil();
  std::string value() const override;
};

}

}

#endif
