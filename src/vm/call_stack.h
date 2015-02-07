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
             vector_ref<instruction>     instr_ptr);

  boost::optional<std::shared_ptr<call_stack> const> parent;
  boost::optional<std::shared_ptr<call_stack> const> enclosing;
  std::unordered_map<symbol, value::base* const> local;
  std::unordered_map<symbol, value::base* const> parameters;
  boost::optional<value::base&> self;

  std::vector<value::base* const> pushed_args;
  boost::optional<value::base&> pushed_self;

  vector_ref<instruction> instr_ptr;
};

}

}

#endif
