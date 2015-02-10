#ifndef VV_GC_H
#define VV_GC_H

#include "value.h"
#include "value/boolean.h"
#include "value/integer.h"
#include "value/nil.h"
#include "vm/call_stack.h"

#include <array>

namespace vv {

namespace gc {

namespace internal {

value::base* emplace(value::base* item);

extern value::nil g_nil;
extern value::boolean g_true;
extern value::boolean g_false;
extern std::array<value::integer, 1024> g_ints;

}

template <typename T, typename... Args>
inline value::base* alloc(Args&&... args)
{
  return internal::emplace(new T{args...});
}

template <>
inline value::base* alloc<value::boolean>(bool&& val)
{
  return &(val ? internal::g_true : internal::g_false);
}

template <>
inline value::base* alloc<value::nil>()
{
  return &internal::g_nil;
}

template <>
inline value::base* alloc<value::integer>(int&& val)
{
  if (val >= 0 && val < 1024)
    return &internal::g_ints[static_cast<unsigned>(val)];
  return gc::alloc<value::integer>( val );
}

void set_current_frame(std::shared_ptr<vm::call_stack> frame);
void set_current_retval(value::base* val);

void init();
void empty();

}

}

#endif
