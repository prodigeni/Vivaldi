#ifndef IL_VM_CALL_STACK_H
#define IL_VM_CALL_STACK_H

#include "instruction.h"

#include "symbol.h"
#include "utils.h"
#include "value.h"

#include <boost/optional/optional.hpp>

#include <unordered_map>

namespace il {

namespace vm {

class call_stack {
public:
  call_stack(std::shared_ptr<call_stack> parent,
             std::shared_ptr<call_stack> enclosing,
             vector_ref<command>     instr_ptr);

  const std::shared_ptr<call_stack> parent;
  const std::shared_ptr<call_stack> enclosing;
  std::unordered_map<symbol, value::base*> local;
  boost::optional<value::base&> self;

  std::vector<value::base*> pushed_args;
  boost::optional<value::base&> pushed_self;

  vector_ref<command> instr_ptr;
};

}

}

#endif
