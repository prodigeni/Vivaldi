#include "call_stack.h"

using namespace il;

vm::call_stack::call_stack(std::shared_ptr<call_stack> new_parent,
                           std::shared_ptr<call_stack> new_enclosing,
                           std::vector<value::base*>&& new_args,
                           vector_ref<command>         new_instr_ptr)
  : parent    {new_parent},
    enclosing {new_enclosing},
    args      {new_args},
    instr_ptr {new_instr_ptr}
{
  if (parent)
    self = parent->pushed_self;
}
