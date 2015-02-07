#ifndef IL_VALUE_BUILTIN_FUNCTION_H
#define IL_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "vm/call_stack.h"

namespace il {

namespace value {

struct builtin_function : public base {
public:
  builtin_function(
      const std::function<base*(std::shared_ptr<vm::call_stack>)>& body);

  std::string value() const override;

  std::function<base*(std::shared_ptr<vm::call_stack>)> body;
};

}

}

#endif
