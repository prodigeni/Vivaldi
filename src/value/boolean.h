#ifndef IL_VALUE_BOOLEAN_H
#define IL_VALUE_BOOLEAN_H

#include "value.h"

namespace il {

namespace value {

struct boolean : public base {
public:
  boolean(bool val);

  std::string value() const override;

  bool val;
};

}

}

#endif
