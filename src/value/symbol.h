#ifndef IL_VALUE_SYMBOL_H
#define IL_VALUE_SYMBOL_H

#include "../symbol.h"
#include "value.h"

namespace il {

namespace value {

struct symbol : public base {
public:
  symbol(il::symbol val);

  std::string value() const override;

  il::symbol val;
};

}

}

#endif
