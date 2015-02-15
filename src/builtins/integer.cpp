#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_function.h"
#include "value/integer.h"
#include "value/floating_point.h"

using namespace vv;
using namespace builtin;
using value::builtin_function;

namespace {

int to_int(value::base* boxed)
{
  return static_cast<value::integer*>(boxed)->val;
}

double to_float(value::base* boxed)
{
  return static_cast<value::floating_point*>(boxed)->val;
}

// integer {{{

template <typename F>
auto fn_int_or_flt_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    auto left = to_int(&*vm.frame->self);
    auto arg = get_arg(vm, 0);
    if (arg->type == &type::floating_point)
      return gc::alloc<value::floating_point>( op(left, to_float(arg)) );

    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer", vm);
    // Apparently moving from integers is a bad idea; without the explicit int&&
    // template parameter, 0 is always passed to value::integer::integer
    return gc::alloc<value::integer, int&&>( op(left, to_int(arg)) );
  };
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    auto left = to_int(&*vm.frame->self);
    if (get_arg(vm, 0)->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer", vm);
    auto right = to_int(get_arg(vm, 0));

    return gc::alloc<value::integer, int&&>( op(left, right) );
  };
}

template <typename F>
auto fn_integer_monop(const F& op)
{
  return [=](vm::machine& vm)
  {
    return gc::alloc<value::integer, int&&>( op(to_int(&*vm.frame->self)) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](vm::machine& vm)
  {

    auto arg = get_arg(vm, 0);
    if (arg->type == &type::floating_point) {
      auto left = to_int(&*vm.frame->self);
      auto right = to_float(arg);
      return gc::alloc<value::boolean>( op(left, right) );
    }
    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer", vm);

    auto left = to_int(&*vm.frame->self);
    auto right = to_int(arg);
    return gc::alloc<value::boolean>( op(left, right) );
  };
}

bool boxed_integer_equal(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::floating_point) {
    auto left = to_int(&*vm.frame->self);
    auto right = to_float(arg);
    return left == right;
  }
  if (arg->type != &type::integer)
    return false;

  auto left = to_int(&*vm.frame->self);
  auto right = to_int(arg);
  return left == right;
}

value::base* fn_integer_equals(vm::machine& vm)
{
  return gc::alloc<value::boolean>( boxed_integer_equal(vm) );
}

value::base* fn_integer_unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !boxed_integer_equal(vm) );
}

value::base* fn_integer_pow(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::floating_point) {
    auto left = to_int(&*vm.frame->self);
    auto right = to_float(arg);
    return gc::alloc<value::floating_point>( pow(left, right) );
  }

  auto left = to_int(&*vm.frame->self);
  if (arg->type != &type::integer)
    return throw_exception("Right-hand argument is not an Integer", vm);
  auto right = to_int(arg);

  if (right < 0)
    return gc::alloc<value::floating_point>( pow(left, right) );
  return gc::alloc<value::integer>( static_cast<int>(pow(left, right)) );
}

// }}}

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
builtin_function int_eq       {fn_integer_equals,                                     1};
builtin_function int_neq      {fn_integer_unequal,                                    1};
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
