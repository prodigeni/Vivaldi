#ifndef IL_VALUE_ARRAY_H
#define IL_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class array : public base {
public:
  array(const std::vector<base*>& mems);

  custom_type* type() const override;
  std::string value() const override;

  base* copy() const override;

private:
  std::vector<base*> m_mems;
};

}

}

#endif
