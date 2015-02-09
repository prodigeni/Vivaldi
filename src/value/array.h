#ifndef VV_VALUE_ARRAY_H
#define VV_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

struct array : public base {
public:
  array(const std::vector<base*>& mems);

  std::string value() const override;
  void mark() override;

  std::vector<base*> mems;
};

}

}

#endif
