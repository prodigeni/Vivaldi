#ifndef IL_VALUE_BOOLEAN_H
#define IL_VALUE_BOOLEAN_H

#include "value.h"

namespace il {

namespace value {

class boolean : public base {
public:
  boolean(bool val) : m_val{val} { }

  basic_type* type() const override;
  std::string value() const override;

  base* copy() const override;

private:
  bool m_val;
};

}

}

#endif
