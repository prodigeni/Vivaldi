#ifndef IL_VALUE_ARRAY_H
#define IL_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class array : public base {
public:
  array(const std::vector<base*>& mems);

  basic_type* type() const override;
  std::string value() const override;

  std::vector<base*>& members() { return m_mems; }
  const std::vector<base*>& members() const { return m_mems; }

  base* copy() const override;

  void mark() override;

private:
  std::vector<base*> m_mems;
};

}

}

#endif
