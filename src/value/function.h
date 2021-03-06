#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value.h"
#include "utils.h"
#include "vm/call_frame.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public base {
  function(int argc,
           const std::vector<vm::command>& body,
           std::shared_ptr<vm::call_frame> enclosure);

  std::string value() const override;

  int argc;
  std::vector<vm::command> body;
  std::shared_ptr<vm::call_frame> enclosure;
};

}

}

#endif
