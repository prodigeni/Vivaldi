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

// TODO: simplify radically; a lot of stuff in here is either redundant,
// inefficient, or just exists as a hack to prevent GC'ing the wrong things
class call_stack {
public:
  call_stack(std::shared_ptr<call_stack> parent,
             std::shared_ptr<call_stack> enclosing,
             size_t                      args,
             vector_ref<command>         instr_ptr);

  /// Frame from which current function was called
  const std::shared_ptr<call_stack> parent;
  /// Frame in which current function (ie closure) was defined
  const std::shared_ptr<call_stack> enclosing;
  /// Local variables
  std::vector<std::unordered_map<symbol, value::base*>> local;
  /// self, if this is a method call
  boost::optional<value::base&> self;

  /// Arguments to be passed in eventual function call, as well as temporaries
  std::vector<value::base*> pushed;
  /// Number of function arguments --- stored in parent's pushed_args
  size_t args;
  /// Self to be passed in eventual method call
  boost::optional<value::base&> pushed_self;

  /// Catch expression provided by try...catch blocks
  boost::optional<value::base&> catcher;
  /// Function from whom the current instruction pointer originates (stored here
  /// solely to avoid GC'ing it)
  boost::optional<value::base&> caller;

  /// Current instruction pointer
  vector_ref<command> instr_ptr;

};

void mark(call_stack& stack);

}

}

#endif
