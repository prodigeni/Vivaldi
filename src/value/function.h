#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value.h"
#include "utils.h"
#include "vm/call_stack.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public base {
  function(const std::vector<vm::command>& body,
           std::shared_ptr<vm::call_stack> enclosure);

  std::string value() const override;

  std::vector<vm::command> body;
  std::shared_ptr<vm::call_stack> enclosure;
};

}

}

#endif
