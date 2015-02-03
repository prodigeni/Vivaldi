#ifndef IL_VALUE_BOOLEAN_H
#define IL_VALUE_BOOLEAN_H

#include "value.h"

namespace il {

namespace value {

class boolean : public base {
public:
  boolean(bool val, environment& env);

  bool bool_val() const { return m_val; }

  std::string value() const override;
  base* copy() const override;

private:
  bool m_val;
};

}

}

#endif
