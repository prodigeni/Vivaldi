#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/integer.h"
#include "value/nil.h"
#include "value/string.h"
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
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);

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

value::base* fn_gets(vm::machine& vm)
{
  if (!check_size(0, vm.stack->args, vm))
    return vm.retval;
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

value::base* fn_quit(vm::machine& vm)
{
  if (!check_size(0, vm.stack->args, vm))
    return vm.retval;
  gc::empty();
  exit(0);
}

value::base* fn_type(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  return pop_arg(vm)->type;
}

}

value::builtin_function function::print{fn_print};
value::builtin_function function::puts{ fn_puts};
value::builtin_function function::gets{ fn_gets};
value::builtin_function function::quit{ fn_quit};
value::builtin_function function::type{ fn_type};

// }}}
// Methods and constructors {{{

namespace {

// Converters {{{

boost::optional<int> to_int(const value::base& boxed) noexcept
{
  if (boxed.type != &type::integer)
    return {};
  return static_cast<const value::integer&>(boxed).val;
}

boost::optional<double> to_float(const value::base& boxed) noexcept
{
  if (boxed.type != &type::floating_point)
    return {};
  return static_cast<const value::floating_point&>(boxed).val;
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

boost::optional<bool> to_bool(const value::base& boxed) noexcept
{
  if (boxed.type != &type::boolean)
    return {};
  return static_cast<const value::boolean&>(boxed).val;
}

// }}}

// array {{{

value::base* fn_array_ctr(vm::machine& vm)
{
  std::vector<value::base*> args;
  while (vm.stack->args)
    args.push_back(pop_arg(vm));
  return gc::alloc<value::array>( args );
}

value::base* fn_array_size(vm::machine& vm)
{
  if (!check_size(0, vm.stack->args, vm))
    return vm.retval;
  auto sz = static_cast<value::array&>(*vm.stack->self).mems.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_array_append(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;

  auto arg = pop_arg(vm);
  if (arg->type == &type::array) {
    auto& arr = static_cast<value::array&>(*vm.stack->self).mems;
    const auto& new_mems = static_cast<value::array*>(arg)->mems;
    copy(begin(new_mems), end(new_mems), back_inserter(arr));
  } else {
    static_cast<value::array&>(*vm.stack->self).mems.push_back(arg);
  }
  return &*vm.stack->self;
}

value::base* fn_array_at(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;

  auto arg = pop_arg(vm);
  if (arg->type != &type::integer)
    return throw_exception("Index must be an Integer", vm);
  auto val = static_cast<value::integer*>(arg)->val;
  const auto& arr = static_cast<value::array&>(*vm.stack->self).mems;
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception("Out of range (expected 0-"
                           + std::to_string(arr.size()) + ", got "
                           + std::to_string(val) + ")",
                           vm);
  return arr[static_cast<unsigned>(val)];
}

// }}}
// integer {{{

value::base* fn_integer_ctr(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);
  auto type = arg->type;

  if (type == &type::integer)
    return arg;

  if (type == &type::floating_point)
    return gc::alloc<value::integer>( static_cast<int>(*to_float(*arg)) );

  return throw_exception("Cannot create Integer from " + arg->value(), vm);
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    if (!check_size(1, vm.stack->args, vm))
      return vm.retval;

    auto left = to_int(*vm.stack->self);
    auto right = to_int(*pop_arg(vm));
    if (!right)
      return throw_exception("Right-hand argument is not an Integer", vm);
    // Apparently moving from integers is a bad idea; without the explicit int&&
    // template parameter, 0 is always passed to value::integer::integer
    return gc::alloc<value::integer, int&&>( op(*left, *right) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    if (!check_size(1, vm.stack->args, vm))
      return vm.retval;

    auto left = to_int(*vm.stack->self);
    auto right = to_int(*pop_arg(vm));
    if (!right)
      return throw_exception("Right-hand argument is not an Integer", vm);
    return gc::alloc<value::boolean>( op(*left, *right) );
  };
}

// }}}
// floating_point {{{

value::base* fn_floating_point_ctr(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);
  auto type = arg->type;

  if (type == &type::floating_point)
    return arg;

  if (type == &type::integer) {
    auto int_val = *to_int(*arg);
    return gc::alloc<value::floating_point>( static_cast<double>(int_val) );
  }

  return throw_exception("Cannot create Float from " + arg->value(), vm);
}

template <typename F>
auto fn_floating_point_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    if (!check_size(1, vm.stack->args, vm))
      return vm.retval;

    auto left = to_float(*vm.stack->self);
    auto right = to_float(*pop_arg(vm));
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
    if (!check_size(1, vm.stack->args, vm))
      return vm.retval;

    auto left = to_float(*vm.stack->self);
    auto right = to_float(*pop_arg(vm));
    if (!right)
      return throw_exception("Right-hand argument is not a Float", vm);
    return gc::alloc<value::boolean>( op(*left, *right) );
  };
}

// }}}
// string {{{

value::base* fn_string_ctr(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);
  if (arg->type == &type::string)
    return gc::alloc<value::string>( *to_string(*arg) );
  if (arg->type == &type::symbol)
    return gc::alloc<value::string>( to_string(*to_symbol(*arg)) );
  return gc::alloc<value::string>( arg->value() );
}

value::base* fn_string_size(
                            vm::machine& vm)
{
  if (!check_size(0, vm.stack->args, vm))
    return vm.retval;
  auto sz = static_cast<value::string&>(*vm.stack->self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_string_equals(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*vm.stack->self) == to_string(*arg) );
}

value::base* fn_string_unequal(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*vm.stack->self) != to_string(*arg) );
}

value::base* fn_string_append(
                              vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto str = to_string(*pop_arg(vm));
  if (!str)
    return throw_exception("Only strings can be appended to other strings", vm);
  static_cast<value::string&>(*vm.stack->self).val += *str;
  return &*vm.stack->self;
}

// }}}
// symbol {{{

value::base* fn_symbol_ctr(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);

  if (arg->type == &type::symbol)
    return arg;
  if (arg->type == &type::string)
    return gc::alloc<value::symbol>( symbol{*to_string(*arg)} );
  return throw_exception(
    "Symbols can only be constructed a String or another Symbol",
    vm);
}

value::base* fn_symbol_equals(
                               vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_symbol(*vm.stack->self) == to_symbol(*arg) );
}

value::base* fn_symbol_unequal(
                               vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(
      to_symbol(*vm.stack->self) != to_symbol(*arg) );
}

value::base* fn_symbol_to_str(
                              vm::machine& vm)
{
  if (!check_size(0, vm.stack->args, vm))
    return vm.retval;
  return gc::alloc<value::string>( to_string(*to_symbol(*vm.stack->self)) );
}

// }}}
// boolean {{{

value::base* fn_bool_ctr(vm::machine& vm)
{
  if (!check_size(1, vm.stack->args, vm))
    return vm.retval;
  auto arg = pop_arg(vm);

  if (arg->type == &type::boolean)
    return arg;
  return gc::alloc<value::boolean>( truthy(arg) );
}

template <typename F>
auto fn_bool_op(const F& op)
{
  return [=](vm::machine& vm)
  {
    if (!check_size(1, vm.stack->args, vm))
      return vm.retval;
    auto left = to_bool(*vm.stack->self);
    auto right = to_bool(*pop_arg(vm));
    if (!right)
      return throw_exception("Right-hand argument is not a Bool", vm);
    return gc::alloc<value::boolean>( op(*left, *right) );
  };
}

// }}}
// custom_type {{{

auto custom_type_default_ctr_maker(value::type* type)
{
  return [=](vm::machine&) { return gc::alloc<value::base>( type ); };
}

value::base* fn_custom_type_ctr(vm::machine& vm)
{
  std::unordered_map<symbol, value::base*> methods;
  for (auto i = vm.stack->args; i--; --i) {
    auto name = to_symbol(*pop_arg(vm));
    if (!name)
      return throw_exception("Type names must be symbols", vm);
    auto definition = pop_arg(vm);
    methods[*name] = definition;
  }

  auto type = gc::alloc<value::type>( nullptr, move(methods) );
  gc::set_current_retval(type);

  auto cast_type = static_cast<value::type*>(type);
  auto constructor = custom_type_default_ctr_maker(cast_type);
  cast_type->constructor = gc::alloc<value::builtin_function>(constructor);

  return type;
}

// }}}

}

// }}}
// Types {{{

using value::builtin_function;

namespace {
builtin_function array_ctr    {fn_array_ctr};
builtin_function array_size   {fn_array_size};
builtin_function array_append {fn_array_append};
builtin_function array_at     {fn_array_at};
}
value::type type::array {&array_ctr, {
  { {"size"},   &array_size },
  { {"append"}, &array_append },
  { {"at"},     &array_at }
}};

namespace {
builtin_function int_ctr            {fn_integer_ctr                           };
builtin_function int_add            {fn_integer_op(std::plus<int>{})          };
builtin_function int_subtract       {fn_integer_op(std::minus<int>{})         };
builtin_function int_times          {fn_integer_op(std::multiplies<int>{})    };
builtin_function int_divides        {fn_integer_op(std::divides<int>{})       };
builtin_function int_modulo         {fn_integer_op(std::modulus<int>{})       };
builtin_function int_bitand         {fn_integer_op(std::bit_and<int>{})       };
builtin_function int_bitor          {fn_integer_op(std::bit_or<int>{})        };
builtin_function int_xor            {fn_integer_op(std::bit_xor<int>{})       };
builtin_function int_equals         {fn_int_bool_op(std::equal_to<int>{})     };
builtin_function int_unequal        {fn_int_bool_op(std::not_equal_to<int>{}) };
builtin_function int_less           {fn_int_bool_op(std::less<int>{})         };
builtin_function int_greater        {fn_int_bool_op(std::greater<int>{})      };
builtin_function int_less_equals    {fn_int_bool_op(std::less_equal<int>{})   };
builtin_function int_greater_equals {fn_int_bool_op(std::greater_equal<int>{})};
}
value::type type::integer{&int_ctr, {
  { {"add"},            &int_add            },
  { {"subtract"},       &int_subtract       },
  { {"times"},          &int_times          },
  { {"divides"},        &int_divides        },
  { {"modulo"},         &int_modulo         },
  { {"bitand"},         &int_bitand         },
  { {"bitor"},          &int_bitor          },
  { {"xor"},            &int_xor            },
  { {"equals"},         &int_equals         },
  { {"unequal"},        &int_unequal        },
  { {"less"},           &int_less           },
  { {"greater"},        &int_greater        },
  { {"less_equals"},    &int_less_equals    },
  { {"greater_equals"}, &int_greater_equals }
} };

namespace {
builtin_function flt_ctr            {fn_floating_point_ctr};
builtin_function flt_add            {fn_floating_point_op(std::plus<double>{})       };
builtin_function flt_subtract       {fn_floating_point_op(std::minus<double>{})      };
builtin_function flt_times          {fn_floating_point_op(std::multiplies<double>{}) };
builtin_function flt_divides        {fn_floating_point_op(std::divides<double>{})    };
builtin_function flt_equals         {fn_float_bool_op(std::equal_to<double>{})       };
builtin_function flt_unequal        {fn_float_bool_op(std::not_equal_to<double>{})   };
builtin_function flt_less           {fn_float_bool_op(std::less<double>{})           };
builtin_function flt_greater        {fn_float_bool_op(std::greater<double>{})        };
builtin_function flt_less_equals    {fn_float_bool_op(std::less_equal<double>{})     };
builtin_function flt_greater_equals {fn_float_bool_op(std::greater_equal<double>{})  };
}
value::type type::floating_point{&flt_ctr, {
  { {"equals"},         &flt_equals         },
  { {"unequal"},        &flt_unequal        },
  { {"add"},            &flt_add            },
  { {"subtract"},       &flt_subtract       },
  { {"times"},          &flt_times          },
  { {"divides"},        &flt_divides        },
  { {"less"},           &flt_less           },
  { {"greater"},        &flt_greater        },
  { {"less_equals"},    &flt_less_equals    },
  { {"greater_equals"}, &flt_greater_equals }
}};

namespace {
builtin_function string_ctr     {fn_string_ctr};
builtin_function string_size    {fn_string_size};
builtin_function string_append  {fn_string_append};
builtin_function string_equals  {fn_string_equals};
builtin_function string_unequal {fn_string_unequal};
}
value::type type::string {&string_ctr, {
  { {"size"},    &string_size    },
  { {"append"},  &string_append  },
  { {"equals"},  &string_equals  },
  { {"unequal"}, &string_unequal }
}};


namespace {
builtin_function symbol_ctr     {fn_symbol_ctr};
builtin_function symbol_equals  {fn_symbol_equals};
builtin_function symbol_unequal {fn_symbol_unequal};
builtin_function symbol_to_str  {fn_symbol_to_str};
}
value::type type::symbol {&symbol_ctr, {
  { {"equals"},  &symbol_equals  },
  { {"unequal"}, &symbol_unequal },
  { {"to_str"},  &symbol_to_str  }
}};

namespace {
builtin_function bool_ctr     {fn_bool_ctr};
builtin_function bool_equals  {fn_bool_op(std::equal_to<bool>{})};
builtin_function bool_unequal {fn_bool_op(std::not_equal_to<bool>{})};
}
value::type type::boolean {&bool_ctr, {
  { {"equals"},  &bool_equals  },
  { {"unequal"}, &bool_unequal }
}};

namespace {
builtin_function custom_type_ctr {fn_custom_type_ctr};
}
value::type type::custom_type {&custom_type_ctr, {
}};

value::type type::nil      {nullptr, { }};
value::type type::function {nullptr, { }};

// }}}

void builtin::make_base_env(vm::call_stack& base)
{
  base.local.back() = {
    { {"print"},   &builtin::function::print },
    { {"puts"},    &builtin::function::puts },
    { {"gets"},    &builtin::function::gets },
    { {"quit"},    &builtin::function::quit },
    { {"type"},    &builtin::function::type },
    { {"Array"},   &builtin::type::array },
    { {"Float"},   &builtin::type::floating_point },
    { {"Integer"}, &builtin::type::integer },
    { {"String"},  &builtin::type::string },
    { {"Bool"},    &builtin::type::boolean },
    { {"Nil"},     &builtin::type::nil },
    { {"Symbol"},  &builtin::type::symbol },
    { {"Type"},    &builtin::type::custom_type }
  };
}
