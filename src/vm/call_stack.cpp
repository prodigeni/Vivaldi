#include "call_stack.h"

using namespace vv;

vm::call_stack::call_stack(std::shared_ptr<call_stack> new_parent,
                           std::shared_ptr<call_stack> new_enclosing,
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

void vm::mark(call_stack& stack)
{
  // Tedious; just call mark on every extant member (unless it's already marked,
  // in which case don't--- both because it's redundant and because of circular
  // references)
  if (stack.parent)
    mark(*stack.parent);
  if (stack.enclosing && stack.parent != stack.enclosing)
    mark(*stack.enclosing);

  for (auto& i : stack.local)
    for (auto& val : i)
      if (!val.second->marked())
        val.second->mark();
  if (stack.self && !stack.self->marked())
    stack.self->mark();

  for (auto* i : stack.pushed_args)
    if (!i->marked())
      i->mark();
  if (stack.catcher && !stack.catcher->marked())
    stack.catcher->mark();
  if (stack.caller && !stack.caller->marked())
    stack.caller->mark();
  if (stack.pushed_self && !stack.pushed_self->marked())
    stack.pushed_self->mark();
}
