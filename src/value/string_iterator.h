#ifndef VV_VALUE_STRING_ITERATOR_H
#define VV_VALUE_STRING_ITERATOR_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

struct string_iterator : public base {
public:
  string_iterator(string& str);
  string_iterator();

  std::string value() const override;
  void mark() override;

  string& str;
  size_t idx;
};

}

}

#endif
