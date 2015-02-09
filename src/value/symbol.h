#ifndef VV_VALUE_SYMBOL_H
#define VV_VALUE_SYMBOL_H

#include "../symbol.h"
#include "value.h"

namespace vv {

namespace value {

struct symbol : public base {
public:
  symbol(vv::symbol val);

  std::string value() const override;

  vv::symbol val;
};

}

}

#endif
