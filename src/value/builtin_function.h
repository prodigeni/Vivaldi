#ifndef VV_VALUE_BUILTIN_FUNCTION_H
#define VV_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "vm.h"

namespace vv {

namespace value {

struct builtin_function : public base {
public:
  builtin_function(
      const std::function<base*(vm::machine&)>& body);

  std::string value() const override;

  std::function<base*(vm::machine&)> body;
};

}

}

#endif
