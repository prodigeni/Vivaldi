#ifndef IL_GC_H
#define IL_GC_H

#include "value.h"

namespace il {

namespace gc {

namespace internal {

void emplace(value::base* item);

}

template <typename T, typename... Args>
T* alloc(Args&&... args)
{
  internal::emplace(new T{std::forward(args...)});
}

}

}

#endif
