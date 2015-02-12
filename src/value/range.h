#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value.h"

namespace vv {

namespace value {

struct range : public base {
  range(value::base& start, value::base& end);
  std::string value() const override;

  value::base* start;
  value::base& end;

  void mark() override;
};

}

}

#endif
