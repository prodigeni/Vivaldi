#ifndef VV_VM_CALL_STACK_H
#define VV_VM_CALL_STACK_H

#include "instruction.h"

#include "symbol.h"
#include "utils.h"
#include "value.h"

#include <boost/optional/optional.hpp>

#include <unordered_map>

namespace vv {

namespace vm {

class call_stack {
public:
  call_stack(std::shared_ptr<call_stack> parent,
             std::shared_ptr<call_stack> enclosing,
             std::vector<value::base*>&& args,
             vector_ref<command>         instr_ptr);

  const std::shared_ptr<call_stack> parent;
  const std::shared_ptr<call_stack> enclosing;
  std::vector<std::unordered_map<symbol, value::base*>> local;
  boost::optional<value::base&> self;

  std::vector<value::base*> args;
  std::vector<value::base*> pushed_args;
  boost::optional<value::base&> pushed_self;

  std::vector<value::base*> catchers;
  boost::optional<value::base&> caller;

  vector_ref<command> instr_ptr;

};

void mark(call_stack& stack);

}

}

#endif
