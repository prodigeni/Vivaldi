#ifndef IL_VALUE_SYMBOL_H
#define IL_VALUE_SYMBOL_H

#include "../symbol.h"
#include "value.h"

namespace il {

namespace value {

class symbol : public base {
public:
  symbol(il::symbol val, environment& env);

  il::symbol sym() const { return m_val; }

  std::string value() const override;
  base* copy() const override;

private:
  il::symbol m_val;
};

}

}

#endif
