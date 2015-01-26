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

}

}

#endif
