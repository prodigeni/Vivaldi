#ifndef IL_VALUE_ARRAY_H
#define IL_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class array : public base {
public:
  array(const std::vector<base*>& mems, environment& env);

  std::vector<base*>& members() { return m_mems; }
  const std::vector<base*>& members() const { return m_mems; }

  std::string value() const override;
  base* copy() const override;

  void mark() override;

private:
  std::vector<base*> m_mems;
};

}

}

#endif
