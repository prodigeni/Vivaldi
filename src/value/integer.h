#ifndef IL_VALUE_INTEGER_H
#define IL_VALUE_INTEGER_H

#include "value.h"

namespace il {

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
