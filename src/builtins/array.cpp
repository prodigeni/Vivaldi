#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

// Array {{{

value::base* fn_array_init(vm::machine& vm)
{
  auto arr = static_cast<value::array*>(&*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::array)
    return throw_exception("Arrays can only be constructed from other Arrays", vm);
  arr->val = static_cast<value::array*>( arg )->val;
  return arr;
}

value::base* fn_array_size(vm::machine& vm)
{
  auto sz = static_cast<value::array&>(*vm.frame->self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_array_append(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::array) {
    auto& arr = static_cast<value::array&>(*vm.frame->self).val;
    const auto& new_val = static_cast<value::array*>(arg)->val;
    copy(begin(new_val), end(new_val), back_inserter(arr));
  } else {
    static_cast<value::array&>(*vm.frame->self).val.push_back(arg);
  }
  return &*vm.frame->self;
}

value::base* fn_array_at(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::integer)
    return throw_exception("Index must be an Integer", vm);
  auto val = static_cast<value::integer*>(arg)->val;
  const auto& arr = static_cast<value::array&>(*vm.frame->self).val;
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception("Out of range (expected 0-"
                           + std::to_string(arr.size()) + ", got "
                           + std::to_string(val) + ")",
                           vm);
  return arr[static_cast<unsigned>(val)];
}

value::base* fn_array_start(vm::machine& vm)
{
  auto& self = static_cast<value::array&>(*vm.frame->self);
  return gc::alloc<value::array_iterator>( self );
}

value::base* fn_array_end(vm::machine& vm)
{
  auto& self = static_cast<value::array&>(*vm.frame->self);
  auto iter = gc::alloc<value::array_iterator>( self );
  static_cast<value::array_iterator*>(iter)->idx = self.val.size();
  return iter;
}

value::base* fn_array_add(vm::machine& vm)
{
  auto arr = static_cast<value::array*>(&*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::array)
    return throw_exception("Only Arrays can be added to other Arrays", vm);
  auto other = static_cast<value::array*>(arg);
  copy(begin(other->val), end(other->val), back_inserter(arr->val));
  return arr;
}

// }}}
// Iterator {{{

value::base* fn_array_iterator_at_start(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  return gc::alloc<value::boolean>( iter->idx == 0 );
}

value::base* fn_array_iterator_at_end(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  return gc::alloc<value::boolean>( iter->idx == iter->arr.val.size() );
}

value::base* fn_array_iterator_get(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  if (iter->idx == iter->arr.val.size())
    return throw_exception("ArrayIterator is at end of array", vm);
  return iter->arr.val[iter->idx];
}

value::base* fn_array_iterator_increment(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  if (iter->idx == iter->arr.val.size())
    return throw_exception("ArrayIterators cannot be incremented past end", vm);
  iter->idx += 1;
  return iter;
}

value::base* fn_array_iterator_decrement(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  if (iter->idx == 0)
    return throw_exception("ArrayIterators cannot be decremented past start", vm);
  iter->idx -= 1;
  return iter;
}

value::base* fn_array_iterator_add(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type != &builtin::type::integer)
    return throw_exception("Only Integers can be added to ArrayIterators", vm);
  auto offset = static_cast<value::integer*>(arg)->val;

  if (static_cast<int>(iter->idx) + offset < 0)
    return throw_exception("ArrayIterators cannot be decremented past start", vm);
  if (iter->idx + offset > iter->arr.val.size())
    return throw_exception("ArrayIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::array_iterator>( *iter );
  static_cast<value::array_iterator*>(other)->idx = iter->idx + offset;
  return other;
}

value::base* fn_array_iterator_subtract(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type != &builtin::type::integer)
    return throw_exception("Only Integers can be added to ArrayIterators", vm);
  auto offset = static_cast<value::integer*>(arg)->val;

  if (!offset)
    return throw_exception("Only numeric types can be added to ArrayIterators", vm);
  if (static_cast<int>(iter->idx) - offset < 0)
    return throw_exception("ArrayIterators cannot be decremented past start", vm);
  if (iter->idx - offset > iter->arr.val.size())
    return throw_exception("ArrayIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::array_iterator>( *iter );
  static_cast<value::array_iterator*>(other)->idx = iter->idx - offset;
  return other;
}

value::base* fn_array_iterator_equals(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  auto other = static_cast<value::array_iterator*>(get_arg(vm, 0));
  return gc::alloc<value::boolean>( &iter->arr == &other->arr
                                  && iter->idx == other->idx );
}

value::base* fn_array_iterator_unequal(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  auto other = static_cast<value::array_iterator*>(get_arg(vm, 0));
  return gc::alloc<value::boolean>( &iter->arr != &other->arr
                                  || iter->idx != other->idx );
}

// }}}

value::builtin_function array_init   {fn_array_init,   1};
value::builtin_function array_size   {fn_array_size,   0};
value::builtin_function array_append {fn_array_append, 1};
value::builtin_function array_at     {fn_array_at,     1};
value::builtin_function array_start  {fn_array_start,  0};
value::builtin_function array_end    {fn_array_end,    0};
value::builtin_function array_add    {fn_array_add,    1};

value::builtin_function array_iterator_at_start  {fn_array_iterator_at_start,  0};
value::builtin_function array_iterator_at_end    {fn_array_iterator_at_end,    0};
value::builtin_function array_iterator_get       {fn_array_iterator_get,       0};
value::builtin_function array_iterator_equals    {fn_array_iterator_equals,    1};
value::builtin_function array_iterator_unequal   {fn_array_iterator_unequal,   1};
value::builtin_function array_iterator_increment {fn_array_iterator_increment, 0};
value::builtin_function array_iterator_decrement {fn_array_iterator_decrement, 0};
value::builtin_function array_iterator_add       {fn_array_iterator_add,       1};
value::builtin_function array_iterator_subtract  {fn_array_iterator_subtract,  1};
}

value::type type::array {gc::alloc<value::array>, {
  { {"init"},   &array_init },
  { {"size"},   &array_size },
  { {"append"}, &array_append },
  { {"at"},     &array_at },
  { {"start"},  &array_start },
  { {"end"},    &array_end },
  { {"add"},    &array_add }
}, builtin::type::object, {"Array"}};

value::type type::array_iterator {[]{ return nullptr; }, {
  { {"at_start"},  &array_iterator_at_start },
  { {"at_end"},    &array_iterator_at_end },
  { {"get"},       &array_iterator_get },
  { {"equals"},    &array_iterator_equals },
  { {"unequal"},   &array_iterator_unequal },
  { {"increment"}, &array_iterator_increment },
  { {"decrement"}, &array_iterator_decrement },
  { {"add"},       &array_iterator_add },
  { {"subtract"},  &array_iterator_subtract },
}, builtin::type::object, {"ArrayIterator"}};
