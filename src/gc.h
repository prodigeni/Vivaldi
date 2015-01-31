#ifndef IL_GC_H
#define IL_GC_H

#include "value.h"

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

// Used to hold unnamed temporaries. Make sure to pop anything you push, and
// *never* use any value::base*'s that haven't either been assigned to an
// environment or pushed here!
// TODO: Implement RAII version
value::base* push_argument(value::base* arg);
void pop_argument();

void init();

}

}

#endif
