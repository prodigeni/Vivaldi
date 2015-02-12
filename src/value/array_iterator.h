#ifndef VV_VALUE_ARRAY_ITERATOR_H
#define VV_VALUE_ARRAY_ITERATOR_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

struct array_iterator : public base {
public:
  array_iterator(array& arr);

  std::string value() const override;
  void mark() override;

  array& arr;
  size_t idx;
};

}

}

#endif
