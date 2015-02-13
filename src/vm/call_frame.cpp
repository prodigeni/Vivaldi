#include "call_frame.h"

using namespace vv;

vm::call_frame::call_frame(std::shared_ptr<call_frame> new_parent,
                           std::shared_ptr<call_frame> new_enclosing,
                           size_t                      new_args,
                           vector_ref<command>         new_instr_ptr)
  : parent    {new_parent},
    enclosing {new_enclosing},
    local     {{}},
    args      {new_args},
    instr_ptr {new_instr_ptr}
{
  if (parent)
    self = parent->pushed_self;
}

void vm::mark(call_frame& frame)
{
  // Tedious; just call mark on every extant member (unless it's already marked,
  // in which case don't--- both because it's redundant and because of circular
  // references)
  if (frame.parent)
    mark(*frame.parent);
  if (frame.enclosing && frame.parent != frame.enclosing)
    mark(*frame.enclosing);

  for (auto& i : frame.local)
    for (auto& val : i)
      if (!val.second->marked())
        val.second->mark();
  if (frame.self && !frame.self->marked())
    frame.self->mark();

  for (auto* i : frame.pushed)
    if (!i->marked())
      i->mark();
  if (frame.catcher && !frame.catcher->marked())
    frame.catcher->mark();
  if (frame.caller && !frame.caller->marked())
    frame.caller->mark();
  if (frame.pushed_self && !frame.pushed_self->marked())
    frame.pushed_self->mark();
}
