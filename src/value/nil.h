#ifndef IL_VALUE_NIL_H
#define IL_VALUE_NIL_H

#include "value.h"

namespace il {

namespace value {

struct nil : public base {
  nil();
  std::string value() const override;
};

}

}

#endif
