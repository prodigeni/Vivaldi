#ifndef VV_VM_CALL_FRAME_H
#define VV_VM_CALL_FRAME_H

#include "instruction.h"

#include "symbol.h"
#include "utils.h"
#include "value.h"

#include <boost/optional/optional.hpp>

#include <unordered_map>

namespace vv {

namespace vm {

// Vivaldi's call 'stack' is something vaguely like a cactus stack. It's
// basically a singly-linked list (the "parent" pointer in the below
// definition), but closures require their own frame pointer, so it ends up
// being a strange sort of tree where each branch connects back to the trunk at
// some point. The only way I've figured out how to memory-manage this
// ridiculous data structure is via reference counting, hence all the
// shared_ptr's below. Unsurprisingly, using this system, the majority of
// execution time is spent allocating or deallocating stack frames. If a better
// approach to closures presents itself, I'll jump on it.

// TODO: simplify radically; a lot of stuff in here is either redundant,
// inefficient, or just exists as a hack to prevent GC'ing the wrong things
class call_frame {
public:
  call_frame(std::shared_ptr<call_frame> parent,
             std::shared_ptr<call_frame> enclosing,
             size_t                      args,
             vector_ref<command>         instr_ptr);

  // Frame from which current function was called
  const std::shared_ptr<call_frame> parent;
  // Frame in which current function (ie closure) was defined
  const std::shared_ptr<call_frame> enclosing;
  // Local variables
  std::vector<std::unordered_map<symbol, value::base*>> local;
  // self, if this is a method call
  boost::optional<value::base&> self;

  // Arguments to be passed in eventual function call, as well as temporaries
  std::vector<value::base*> pushed;
  // Number of function arguments --- stored in parent's pushed_args
  size_t args;
  // Self to be passed in eventual method call
  boost::optional<value::base&> pushed_self;

  // Catch expression provided by try...catch blocks
  boost::optional<value::base&> catcher;
  // Function from whom the current instruction pointer originates (stored here
  // solely to avoid GC'ing it)
  boost::optional<value::base&> caller;

  // Current instruction pointer
  vector_ref<command> instr_ptr;

};

void mark(call_frame& frame);

}

}

#endif
