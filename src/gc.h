#ifndef IL_GC_H
#define IL_GC_H

#include "value.h"
#include "vm/call_stack.h"

namespace il {

namespace gc {

namespace internal {

value::base* emplace(value::base* item);

}

template <typename T, typename... Args>
value::base* alloc(Args&&... args)
{
  return internal::emplace(new T{args...});
}

void set_current_frame(std::shared_ptr<vm::call_stack> frame);
void set_current_retval(value::base*& val);

void init();
void empty();

}

}

#endif
