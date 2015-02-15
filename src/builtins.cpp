#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/integer.h"
#include "value/nil.h"
#include "value/range.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"

#include <iostream>

using namespace vv;
using namespace builtin;

// Symbols {{{

vv::symbol sym::self{"self"};
vv::symbol sym::call{"call"};

// }}}
// Freestanding functions {{{

namespace {

value::base* fn_print(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type == &type::string)
    std::cout << static_cast<value::string*>(arg)->val;
  else
    std::cout << arg->value();
  return gc::alloc<value::nil>( );
}

value::base* fn_puts(vm::machine& vm)
{
  auto ret = fn_print(vm);
  std::cout << '\n';
  return ret;
}

value::base* fn_gets(vm::machine&)
{
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

value::base* fn_quit(vm::machine&)
{
  gc::empty();
  exit(0);
}

}

value::builtin_function function::print{fn_print, 1};
value::builtin_function function::puts{ fn_puts,  1};
value::builtin_function function::gets{ fn_gets,  0};
value::builtin_function function::quit{ fn_quit,  0};

// }}}
// Methods and constructors {{{

namespace {

// Converters {{{

boost::optional<int> to_int(const value::base& boxed) noexcept
{
  if (boxed.type == &type::integer)
    return static_cast<const value::integer&>(boxed).val;
  if (boxed.type == &type::floating_point)
      return static_cast<const value::floating_point&>(boxed).val;
  return {};
}

boost::optional<double> to_float(const value::base& boxed) noexcept
{
  if (boxed.type == &type::floating_point)
    return static_cast<const value::floating_point&>(boxed).val;
  if (boxed.type == &type::integer)
    return static_cast<const value::integer&>(boxed).val;
  return {};
}

boost::optional<const std::string&> to_string(const value::base& boxed)
{
  if (boxed.type != &type::string)
    return {};
  return static_cast<const value::string&>(boxed).val;
}

boost::optional<vv::symbol> to_symbol(const value::base& boxed)
{
  if (boxed.type != &type::symbol)
    return {};
  return static_cast<const value::symbol&>(boxed).val;
}

// }}}

// array {{{

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

// }}}
// array_iterator {{{

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
  auto offset = to_int(*get_arg(vm, 0));

  if (!offset)
    return throw_exception("Only numeric types can be added to ArrayIterators", vm);
  if (static_cast<int>(iter->idx) + *offset < 0)
    return throw_exception("ArrayIterators cannot be decremented past start", vm);
  if (iter->idx + *offset > iter->arr.val.size())
    return throw_exception("ArrayIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::array_iterator>( *iter );
  static_cast<value::array_iterator*>(other)->idx = iter->idx + *offset;
  return other;
}

value::base* fn_array_iterator_subtract(vm::machine& vm)
{
  auto iter = static_cast<value::array_iterator*>(&*vm.frame->self);
  auto offset = to_int(*get_arg(vm, 0));

  if (!offset)
    return throw_exception("Only numeric types can be added to ArrayIterators", vm);
  if (static_cast<int>(iter->idx) - *offset < 0)
    return throw_exception("ArrayIterators cannot be decremented past start", vm);
  if (iter->idx - *offset > iter->arr.val.size())
    return throw_exception("ArrayIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::array_iterator>( *iter );
  static_cast<value::array_iterator*>(other)->idx = iter->idx - *offset;
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
// boolean {{{

value::base* fn_bool_init(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::boolean)
    return arg;
  return gc::alloc<value::boolean>( truthy(arg) );
}

// }}}
// custom_type {{{

value::base* fn_custom_type_parent(vm::machine& vm)
{
  return &static_cast<value::type&>(*vm.frame->self).parent;
}

// }}}
// integer {{{

template <typename F>
auto fn_int_or_flt_op(const F& op)
{
  return [=](vm::machine& vm)
  {

    auto arg = get_arg(vm, 0);
    if (arg->type == &type::floating_point) {
      auto left = *to_float(*vm.frame->self);
      auto right = *to_float(*arg);
      return gc::alloc<value::floating_point>( op(left, right) );
    }

    auto left = to_int(*vm.frame->self);
    auto right = to_int(*arg);
    if (!right)
      return throw_exception("Right-hand argument is not an Integer", vm);
    // Apparently moving from integers is a bad idea; without the explicit int&&
    // template parameter, 0 is always passed to value::integer::integer
    return gc::alloc<value::integer, int&&>( op(*left, *right) );
  };
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](vm::machine& vm)
  {

    auto left = to_int(*vm.frame->self);
    auto right = to_int(*get_arg(vm, 0));
    if (!right)
      return throw_exception("Right-hand argument is not an Integer", vm);
    // Apparently moving from integers is a bad idea; without the explicit int&&
    // template parameter, 0 is always passed to value::integer::integer
    return gc::alloc<value::integer, int&&>( op(*left, *right) );
  };
}

template <typename F>
auto fn_integer_monop(const F& op)
{
  return [=](vm::machine& vm)
  {
    return gc::alloc<value::integer, int&&>( op(*to_int(*vm.frame->self)) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](vm::machine& vm)
  {

    auto arg = get_arg(vm, 0);
    if (arg->type == &type::floating_point) {
      auto left = *to_float(*vm.frame->self);
      auto right = *to_float(*arg);
      return gc::alloc<value::boolean>( op(left, right) );
    }

    auto left = to_int(*vm.frame->self);
    auto right = to_int(*arg);
    if (!right)
      return throw_exception("Right-hand argument is not an Integer", vm);
    return gc::alloc<value::boolean>( op(*left, *right) );
  };
}

value::base* fn_integer_pow(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::floating_point) {
    auto left = *to_float(*vm.frame->self);
    auto right = *to_float(*arg);
    return gc::alloc<value::floating_point>( pow(left, right) );
  }

  auto left = *to_int(*vm.frame->self);
  auto right = to_int(*arg);
  if (!right)
    return throw_exception("Right-hand argument is not an Integer", vm);

  if (*right < 0)
    return gc::alloc<value::floating_point>( pow(left, *right) );
  return gc::alloc<value::integer>( static_cast<int>(pow(left, *right)) );
}

// }}}
// floating_point {{{

template <typename F>
auto fn_floating_point_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    auto left = to_float(*vm.frame->self);
    auto right = to_float(*get_arg(vm, 0));
    if (!right)
      return throw_exception("Right-hand argument is not a Float", vm);
    return gc::alloc<value::floating_point>( op(*left, *right) );
  };
}

template <typename F>
auto fn_float_bool_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    auto left = to_float(*vm.frame->self);
    auto right = to_float(*get_arg(vm, 0));
    if (!right)
      return throw_exception("Right-hand argument is not a Float", vm);
    return gc::alloc<value::boolean>( op(*left, *right) );
  };
}

value::base* fn_floating_point_negative(vm::machine& vm)
{
  return gc::alloc<value::floating_point>( -*to_float(*vm.frame->self) );
}

// }}}
// object {{{

value::base* fn_object_equals(vm::machine& vm)
{
  return gc::alloc<value::boolean>( &*vm.frame->self == get_arg(vm, 0) );
}

value::base* fn_object_unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( &*vm.frame->self != get_arg(vm, 0) );
}

value::base* fn_object_not(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !truthy(&*vm.frame->self) );
}

value::base* fn_object_type(vm::machine& vm)
{
  return vm.frame->self->type;
}

// }}}
// range {{{


value::base* fn_range_init(vm::machine& vm)
{
  auto& rng = static_cast<value::range&>(*vm.frame->self);
  rng.end = get_arg(vm, 1);
  rng.start = get_arg(vm, 0);
  return &rng;
}

value::base* fn_range_start(vm::machine& vm)
{
  return &*vm.frame->self;
}

value::base* fn_range_size(vm::machine& vm)
{
  auto& rng = static_cast<value::range&>(*vm.frame->self);
  vm.retval = rng.start;
  vm.push_arg();
  vm.retval = rng.end;
  vm.readm({"subtract"});
  vm.call(1);
  return vm.retval;
}

value::base* fn_range_at_end(vm::machine& vm)
{
  auto& rng = static_cast<value::range&>(*vm.frame->self);
  vm.retval = rng.start;
  vm.push_arg();
  vm.retval = rng.end;
  vm.readm({"greater"});
  vm.call(1);
  vm.readm({"not"});
  vm.call(0);
  return vm.retval;
}

value::base* fn_range_get(vm::machine& vm)
{
  return static_cast<value::range&>(*vm.frame->self).start;
}

value::base* fn_range_increment(vm::machine& vm)
{
  auto& rng = static_cast<value::range&>(*vm.frame->self);
  vm.push_int(1);
  vm.push_arg();
  vm.retval = rng.start;
  vm.readm({"add"});
  vm.call(1);
  rng.start = vm.retval;
  return &rng;
}

// }}}
// string {{{

value::base* fn_string_init(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::string)
    return gc::alloc<value::string>( *to_string(*arg) );
  if (arg->type == &type::symbol)
    return gc::alloc<value::string>( to_string(*to_symbol(*arg)) );
  return gc::alloc<value::string>( arg->value() );
}

value::base* fn_string_size(vm::machine& vm)
{
  auto sz = static_cast<value::string&>(*vm.frame->self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_string_equals(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*vm.frame->self) == to_string(*arg) );
}

value::base* fn_string_unequal(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*vm.frame->self) != to_string(*arg) );
}

value::base* fn_string_append(vm::machine& vm)
{
  auto str = to_string(*get_arg(vm, 0));
  if (!str)
    return throw_exception("Only strings can be appended to other strings", vm);
  static_cast<value::string&>(*vm.frame->self).val += *str;
  return &*vm.frame->self;
}

value::base* fn_string_add(vm::machine& vm)
{
  auto str = to_string(*get_arg(vm, 0));
  if (!str)
    return throw_exception("Only Strings can be appended to other Strings", vm);
  auto new_str = static_cast<value::string&>(*vm.frame->self).val + *str;
  return gc::alloc<value::string>( new_str );
}

value::base* fn_string_times(vm::machine& vm)
{
  auto times = to_int(*get_arg(vm, 0));
  if (!times)
    return throw_exception("Strings can only be multiplied by numbers", vm);
  auto val = static_cast<value::string&>(*vm.frame->self).val;
  std::string new_str{};
  for (auto i = *times; i--;)
    new_str += val;
  return gc::alloc<value::string>( new_str );
}

// }}}
// string_iterator {{{

value::base* fn_string_iterator_at_start(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  return gc::alloc<value::boolean>( iter->idx == 0 );
}

value::base* fn_string_iterator_at_end(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  return gc::alloc<value::boolean>( iter->idx == iter->str.val.size() );
}

value::base* fn_string_iterator_get(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  if (iter->idx == iter->str.val.size())
    return throw_exception("StringIterator is at end of string", vm);
  return gc::alloc<value::string>( std::string{iter->str.val[iter->idx]} );
}

value::base* fn_string_iterator_increment(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  if (iter->idx == iter->str.val.size())
    return throw_exception("StringIterators cannot be incremented past end", vm);
  iter->idx += 1;
  return iter;
}

value::base* fn_string_iterator_decrement(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  if (iter->idx == 0)
    return throw_exception("StringIterators cannot be decremented past start", vm);
  iter->idx -= 1;
  return iter;
}

value::base* fn_string_iterator_add(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  auto offset = to_int(*get_arg(vm, 0));

  if (!offset)
    return throw_exception("Only numeric types can be added to StringIterators", vm);
  if (static_cast<int>(iter->idx) + *offset < 0)
    return throw_exception("StringIterators cannot be decremented past start", vm);
  if (iter->idx + *offset > iter->str.val.size())
    return throw_exception("StringIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::string_iterator>( *iter );
  static_cast<value::string_iterator*>(other)->idx = iter->idx + *offset;
  return other;
}

value::base* fn_string_iterator_subtract(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  auto offset = to_int(*get_arg(vm, 0));

  if (!offset)
    return throw_exception("Only numeric types can be added to StringIterators", vm);
  if (static_cast<int>(iter->idx) - *offset < 0)
    return throw_exception("StringIterators cannot be decremented past start", vm);
  if (static_cast<int>(iter->idx) - *offset > static_cast<int>(iter->str.val.size()))
    return throw_exception("StringIterators cannot be incremented past end", vm);

  auto other = gc::alloc<value::string_iterator>( *iter );
  static_cast<value::string_iterator*>(other)->idx = iter->idx - *offset;
  return other;
}

value::base* fn_string_iterator_equals(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  auto other = static_cast<value::string_iterator*>(get_arg(vm, 0));
  return gc::alloc<value::boolean>( &iter->str == &other->str
                                  && iter->idx == other->idx );
}

value::base* fn_string_iterator_unequal(vm::machine& vm)
{
  auto iter = static_cast<value::string_iterator*>(&*vm.frame->self);
  auto other = static_cast<value::string_iterator*>(get_arg(vm, 0));
  return gc::alloc<value::boolean>( &iter->str != &other->str
                                  || iter->idx != other->idx );
}

// }}}
// symbol {{{

value::base* fn_symbol_init(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type == &type::symbol)
    return arg;
  if (arg->type == &type::string)
    return gc::alloc<value::symbol>( symbol{*to_string(*arg)} );
  return throw_exception(
    "Symbols can only be constructed a String or another Symbol",
    vm);
}

value::base* fn_symbol_equals(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_symbol(*vm.frame->self) == to_symbol(*arg) );
}

value::base* fn_symbol_unequal(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(
      to_symbol(*vm.frame->self) != to_symbol(*arg) );
}

value::base* fn_symbol_to_str(vm::machine& vm)
{
  return gc::alloc<value::string>( to_string(*to_symbol(*vm.frame->self)) );
}

// }}}

}

// }}}
// Types {{{

using value::builtin_function;

namespace {
builtin_function obj_equals  {fn_object_equals,  1};
builtin_function obj_unequal {fn_object_unequal, 1};
builtin_function obj_not     {fn_object_not,     0};
builtin_function obj_type    {fn_object_type,    0};
}
value::type type::object {gc::alloc<value::base>, {
  { {"equals"},  &obj_equals },
  { {"unequal"}, &obj_unequal },
  { {"not"},     &obj_not },
  { {"type"},    &obj_type }
}, builtin::type::object, {"Object"}};

namespace {
builtin_function array_init   {fn_array_init,   1};
builtin_function array_size   {fn_array_size,   0};
builtin_function array_append {fn_array_append, 1};
builtin_function array_at     {fn_array_at,     1};
builtin_function array_start  {fn_array_start,  0};
builtin_function array_end    {fn_array_end,    0};
}
value::type type::array {gc::alloc<value::array>, {
  { {"init"},   &array_init },
  { {"size"},   &array_size },
  { {"append"}, &array_append },
  { {"at"},     &array_at },
  { {"start"},  &array_start },
  { {"end"},    &array_end },
}, builtin::type::object, {"Array"}};

namespace {
builtin_function array_iterator_at_start  {fn_array_iterator_at_start,  0};
builtin_function array_iterator_at_end    {fn_array_iterator_at_end,    0};
builtin_function array_iterator_get       {fn_array_iterator_get,       0};
builtin_function array_iterator_equals    {fn_array_iterator_equals,    1};
builtin_function array_iterator_unequal   {fn_array_iterator_unequal,   1};
builtin_function array_iterator_increment {fn_array_iterator_increment, 0};
builtin_function array_iterator_decrement {fn_array_iterator_decrement, 0};
builtin_function array_iterator_add       {fn_array_iterator_add,       1};
builtin_function array_iterator_subtract  {fn_array_iterator_subtract,  1};
}
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

namespace {
builtin_function int_add      {fn_int_or_flt_op([](auto a, auto b){ return a + b; }), 1};
builtin_function int_subtract {fn_int_or_flt_op([](auto a, auto b){ return a - b; }), 1};
builtin_function int_times    {fn_int_or_flt_op([](auto a, auto b){ return a * b; }), 1};
builtin_function int_divides  {fn_int_or_flt_op([](auto a, auto b){ return a / b; }), 1};
builtin_function int_modulo   {fn_integer_op(std::modulus<int>{}),                    1};
builtin_function int_pow      {fn_integer_pow,                                        1};
builtin_function int_lshift   {fn_integer_op([](int a, int b) { return a << b; }),    1};
builtin_function int_rshift   {fn_integer_op([](int a, int b) { return a >> b; }),    1};
builtin_function int_bitand   {fn_integer_op(std::bit_and<int>{}),                    1};
builtin_function int_bitor    {fn_integer_op(std::bit_or<int>{}),                     1};
builtin_function int_xor      {fn_integer_op(std::bit_xor<int>{}),                    1};
builtin_function int_eq       {fn_int_bool_op([](auto a, auto b){ return a == b; }),  1};
builtin_function int_neq      {fn_int_bool_op([](auto a, auto b){ return a != b; }),  1};
builtin_function int_lt       {fn_int_bool_op([](auto a, auto b){ return a < b;  }),  1};
builtin_function int_gt       {fn_int_bool_op([](auto a, auto b){ return a > b;  }),  1};
builtin_function int_le       {fn_int_bool_op([](auto a, auto b){ return a <= b; }),  1};
builtin_function int_ge       {fn_int_bool_op([](auto a, auto b){ return a >= b; }),  1};
builtin_function int_negative {fn_integer_monop(std::negate<int>{}),                  0};
builtin_function int_negate   {fn_integer_monop(std::bit_not<int>{}),                 0};
}
value::type type::integer{[]{ return nullptr; }, {
  { {"add"},            &int_add      },
  { {"subtract"},       &int_subtract },
  { {"times"},          &int_times    },
  { {"divides"},        &int_divides  },
  { {"modulo"},         &int_modulo   },
  { {"pow"},            &int_pow      },
  { {"bitand"},         &int_bitand   },
  { {"bitor"},          &int_bitor    },
  { {"xor"},            &int_xor      },
  { {"lshift"},         &int_lshift   },
  { {"rshift"},         &int_rshift   },
  { {"equals"},         &int_eq       },
  { {"unequal"},        &int_neq      },
  { {"less"},           &int_lt       },
  { {"greater"},        &int_gt       },
  { {"less_equals"},    &int_le       },
  { {"greater_equals"}, &int_ge       },
  { {"negative"},       &int_negative },
  { {"negate"},         &int_negate   }
}, builtin::type::object, {"Integer"}};

namespace {
builtin_function flt_add      {fn_floating_point_op(std::plus<double>{}),       1};
builtin_function flt_subtract {fn_floating_point_op(std::minus<double>{}),      1};
builtin_function flt_times    {fn_floating_point_op(std::multiplies<double>{}), 1};
builtin_function flt_divides  {fn_floating_point_op(std::divides<double>{}),    1};
builtin_function flt_pow      {fn_floating_point_op(pow),                       1};
builtin_function flt_eq       {fn_float_bool_op(std::equal_to<double>{}),       1};
builtin_function flt_neq      {fn_float_bool_op(std::not_equal_to<double>{}),   1};
builtin_function flt_lt       {fn_float_bool_op(std::less<double>{}),           1};
builtin_function flt_gt       {fn_float_bool_op(std::greater<double>{}),        1};
builtin_function flt_le       {fn_float_bool_op(std::less_equal<double>{}),     1};
builtin_function flt_ge       {fn_float_bool_op(std::greater_equal<double>{}),  1};
builtin_function flt_negative {fn_floating_point_negative,                      0};
}
value::type type::floating_point{[]{ return nullptr; }, {
  { {"add"},            &flt_add      },
  { {"subtract"},       &flt_subtract },
  { {"times"},          &flt_times    },
  { {"divides"},        &flt_divides  },
  { {"pow"},            &flt_pow      },
  { {"equals"},         &flt_eq       },
  { {"unequal"},        &flt_neq      },
  { {"less"},           &flt_lt       },
  { {"greater"},        &flt_gt       },
  { {"less_equals"},    &flt_le       },
  { {"greater_equals"}, &flt_ge       },
  { {"negative"},       &flt_negative }
}, builtin::type::object, {"Float"}};

namespace {
builtin_function range_init      {fn_range_init,      2};
builtin_function range_start     {fn_range_start,     0};
builtin_function range_size      {fn_range_size,      0};
builtin_function range_at_end    {fn_range_at_end,    0};
builtin_function range_get       {fn_range_get,       0};
builtin_function range_increment {fn_range_increment, 0};
}
value::type type::range {gc::alloc<value::range>, {
  { {"init"},      &range_init },
  { {"start"},     &range_start },
  { {"size"},      &range_size },
  { {"at_end"},    &range_at_end },
  { {"get"},       &range_get },
  { {"increment"}, &range_increment }
}, builtin::type::object, {"Range"}};

namespace {
builtin_function string_init    {fn_string_init,    1};
builtin_function string_size    {fn_string_size,    0};
builtin_function string_append  {fn_string_append,  1};
builtin_function string_equals  {fn_string_equals,  1};
builtin_function string_unequal {fn_string_unequal, 1};
builtin_function string_add     {fn_string_add,     1};
builtin_function string_times   {fn_string_times,   1};
}
value::type type::string {gc::alloc<value::string>, {
  { {"init"},    &string_init    },
  { {"size"},    &string_size    },
  { {"append"},  &string_append  },
  { {"equals"},  &string_equals  },
  { {"unequal"}, &string_unequal },
  { {"add"},     &string_add     },
  { {"times"},   &string_times   }
}, builtin::type::object, {"String"}};

namespace {
builtin_function string_iterator_at_start  {fn_string_iterator_at_start,  0};
builtin_function string_iterator_at_end    {fn_string_iterator_at_end,    0};
builtin_function string_iterator_get       {fn_string_iterator_get,       0};
builtin_function string_iterator_equals    {fn_string_iterator_equals,    1};
builtin_function string_iterator_unequal   {fn_string_iterator_unequal,   1};
builtin_function string_iterator_increment {fn_string_iterator_increment, 0};
builtin_function string_iterator_decrement {fn_string_iterator_decrement, 0};
builtin_function string_iterator_add       {fn_string_iterator_add,       1};
builtin_function string_iterator_subtract  {fn_string_iterator_subtract,  1};
}
value::type type::string_iterator {[]{ return nullptr; }, {
  { {"at_start"},  &string_iterator_at_start  },
  { {"at_end"},    &string_iterator_at_end    },
  { {"get"},       &string_iterator_get       },
  { {"equals"},    &string_iterator_equals    },
  { {"unequal"},   &string_iterator_unequal   },
  { {"increment"}, &string_iterator_increment },
  { {"decrement"}, &string_iterator_decrement },
  { {"add"},       &string_iterator_add       },
  { {"subtract"},  &string_iterator_subtract  },
}, builtin::type::object, {"StringIterator"}};

namespace {
builtin_function symbol_init    {fn_symbol_init,    1};
builtin_function symbol_equals  {fn_symbol_equals,  1};
builtin_function symbol_unequal {fn_symbol_unequal, 1};
builtin_function symbol_to_str  {fn_symbol_to_str,  0};
}
value::type type::symbol {gc::alloc<value::symbol>, {
  { {"init"},    &symbol_init    },
  { {"equals"},  &symbol_equals  },
  { {"unequal"}, &symbol_unequal },
  { {"to_str"},  &symbol_to_str  }
}, builtin::type::object, {"Symbol"}};

namespace {
builtin_function bool_init {fn_bool_init, 1};
}
value::type type::boolean {gc::alloc<value::boolean>, {
  { {"init"}, &bool_init }
}, builtin::type::object, {"Bool"}};

namespace {
builtin_function custom_type_parent {fn_custom_type_parent, 0};
}
value::type type::custom_type {[]{ return nullptr; }, {
  { {"parent"}, &custom_type_parent }
}, builtin::type::object, {"Type"}};

value::type type::nil      {[]{ return nullptr; }, {
}, builtin::type::object, {"Nil"}};

value::type type::function {[]{ return nullptr; }, {
}, builtin::type::object, {"Function"}};

// }}}

void builtin::make_base_env(vm::call_frame& base)
{
  base.local.back() = {
    { {"print"},          &builtin::function::print },
    { {"puts"},           &builtin::function::puts },
    { {"gets"},           &builtin::function::gets },
    { {"quit"},           &builtin::function::quit },
    { {"Array"},          &builtin::type::array },
    { {"ArrayIterator"},  &builtin::type::array_iterator },
    { {"Bool"},           &builtin::type::boolean },
    { {"Float"},          &builtin::type::floating_point },
    { {"Integer"},        &builtin::type::integer },
    { {"Nil"},            &builtin::type::nil },
    { {"Object"},         &builtin::type::object },
    { {"Range"},          &builtin::type::range },
    { {"String"},         &builtin::type::string },
    { {"StringIterator"}, &builtin::type::string_iterator },
    { {"Symbol"},         &builtin::type::symbol },
    { {"Type"},           &builtin::type::custom_type }
  };
}
