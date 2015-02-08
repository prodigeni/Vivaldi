#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/builtin_type.h"
#include "value/custom_type.h"
#include "value/floating_point.h"
#include "value/integer.h"
#include "value/nil.h"
#include "value/string.h"
#include "value/symbol.h"

#include <iostream>

using namespace il;
using namespace builtin;

// Symbols {{{

il::symbol sym::self{"self"};
il::symbol sym::call{"call"};

// }}}
// Freestanding functions {{{

namespace {

value::base* fn_print(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type == &type::string)
    std::cout << static_cast<value::string*>(base.args.front())->val;
  else
    std::cout << base.args.front()->value();
  return gc::alloc<value::nil>( );
}

value::base* fn_puts(vm::call_stack& base)
{
  auto ret = fn_print(base);
  std::cout << '\n';
  return ret;
}

value::base* fn_gets(vm::call_stack& base)
{
  check_size(0, base.args.size());
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

[[noreturn]]
value::base* fn_quit(vm::call_stack& base)
{
  check_size(0, base.args.size());
  gc::empty();
  exit(0);
}

value::base* fn_type(vm::call_stack& base)
{
  check_size(1, base.args.size());
  return base.args.front()->type;
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

int to_int(const value::base& boxed)
{
  if (boxed.type != &type::integer)
    throw std::runtime_error{"argument must be an integer"};
  return static_cast<const value::integer&>(boxed).val;
}

double to_float(const value::base& boxed)
{
  if (boxed.type != &type::floating_point)
    throw std::runtime_error{"argument must be a float"};
  return static_cast<const value::floating_point&>(boxed).val;
}

const std::string& to_string(const value::base& boxed)
{
  if (boxed.type != &type::string)
    throw std::runtime_error{"argument must be a string"};
  return static_cast<const value::string&>(boxed).val;
}

il::symbol to_symbol(const value::base& boxed)
{
  if (boxed.type != &type::symbol)
    throw std::runtime_error{"argument must be a symbol"};
  return static_cast<const value::symbol&>(boxed).val;
}

bool to_bool(const value::base& boxed)
{
  if (boxed.type != &type::boolean)
    throw std::runtime_error{"argument must be a symbol"};
  return static_cast<const value::boolean&>(boxed).val;
}

// }}}

// array {{{

value::base* fn_array_ctr(vm::call_stack& base)
{
  return gc::alloc<value::array>( base.args );
}

value::base* fn_array_size(vm::call_stack& base)
{
  check_size(0, base.args.size());
  auto sz = static_cast<value::array&>(*base.self).mems.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_array_append(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type == &type::array) {
    auto& arr = static_cast<value::array&>(*base.self).mems;
    const auto& new_mems = static_cast<value::array*>(base.args.front())->mems;
    copy(begin(new_mems), end(new_mems), back_inserter(arr));
  } else {
    static_cast<value::array&>(*base.self).mems.push_back(base.args.front());
  }
  return &*base.self;
}

value::base* fn_array_at(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::integer)
    throw std::runtime_error{"index must be an integer"};
  auto val = static_cast<value::integer*>(base.args.front())->val;
  const auto& arr = static_cast<value::array&>(*base.self).mems;
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    throw std::runtime_error{"out of range (expected 0-"
                           + std::to_string(arr.size()) + ", got "
                           + std::to_string(val) + ")"};
  return arr.at(static_cast<unsigned>(val));
}

// }}}
// integer {{{

value::base* fn_integer_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  auto type = base.args.front()->type;

  if (type == &type::integer)
    return base.args.front();

  if (type == &type::floating_point)
    return gc::alloc<value::integer>(
        static_cast<int>(to_float(*base.args.front())) );

  throw std::runtime_error{"cannot create Integer from "
                          + base.args.front()->value()};
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](vm::call_stack& base)
  {
    check_size(1, base.args.size());
    return gc::alloc<value::integer>(
        op(to_int(*base.self), to_int(*base.args.front())));
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](vm::call_stack& base)
  {
    check_size(1, base.args.size());
    return gc::alloc<value::boolean>(
        op(to_int(*base.self), to_int(*base.args.front())) );
  };
}

// }}}
// floating_point {{{

value::base* fn_floating_point_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  auto type = base.args.front()->type;

  if (type == &type::floating_point)
    return base.args.front();

  if (type == &type::integer) {
    auto int_val = to_int(*base.args.front());
    return gc::alloc<value::floating_point>( static_cast<double>(int_val) );
  }

  throw std::runtime_error{"cannot create Float from " + base.args.front()->value()};
}

value::base* fn_floating_point_equals(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::floating_point)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_float(*base.self) == to_float(*base.args.front()) );
}

value::base* fn_floating_point_unequal(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::floating_point)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_float(*base.self) != to_float(*base.args.front()) );
}

template <typename F>
auto fn_floating_point_op(const F& op)
{
  return [=](vm::call_stack& base)
  {
    check_size(1, base.args.size());
    return gc::alloc<value::floating_point>( op(to_float(*base.self),
                                                to_float(*base.args.front())) );
  };
}

template <typename F>
auto fn_float_bool_op(const F& op)
{
  return [=](vm::call_stack& base)
  {
    check_size(1, base.args.size());
    return gc::alloc<value::boolean>( op(to_float(*base.self),
                                         to_float(*base.args.front())) );
  };
}

// }}}
// string {{{

value::base* fn_string_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type == &type::string)
    return gc::alloc<value::string>( to_string(*base.args.front()) );
  if (base.args.front()->type == &type::symbol)
    return gc::alloc<value::string>( to_string(to_symbol(*base.args.front())) );
  return gc::alloc<value::string>( base.args.front()->value() );
}

value::base* fn_string_size(
                            vm::call_stack& base)
{
  check_size(0, base.args.size());
  auto sz = static_cast<value::string&>(*base.self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_string_equals(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*base.self) == to_string(*base.args.front()) );
}

value::base* fn_string_unequal(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_string(*base.self) != to_string(*base.args.front()) );
}

value::base* fn_string_append(
                              vm::call_stack& base)
{
  check_size(1, base.args.size());
  auto str = to_string(*base.args.front());
  static_cast<value::string&>(*base.self).val += str;
  return &*base.self;
}

// }}}
// symbol {{{

value::base* fn_symbol_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type == &type::symbol)
    return base.args.front();
  if (base.args.front()->type == &type::string)
    return gc::alloc<value::symbol>( symbol{to_string(*base.args.front())} );
  throw std::runtime_error {
    "can only construct a Symbol from a String or another Symbol"
  };
}

value::base* fn_symbol_equals(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(
      to_symbol(*base.self) == to_symbol(*base.args.front()) );
}

value::base* fn_symbol_unequal(
                               vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(
      to_symbol(*base.self) != to_symbol(*base.args.front()) );
}

value::base* fn_symbol_to_str(
                              vm::call_stack& base)
{
  check_size(0, base.args.size());
  return gc::alloc<value::string>( to_string(to_symbol(*base.self)) );
}

// }}}
// boolean {{{

value::base* fn_bool_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  auto type = base.args.front()->type;

  if (type == &type::boolean)
    return base.args.front();

  return gc::alloc<value::boolean>( truthy(base.args.front()) );
}

template <typename F>
auto fn_bool_op(const F& op)
{
  return [=](vm::call_stack& base)
  {
    check_size(1, base.args.size());
    return gc::alloc<value::boolean>(
        op(to_bool(*base.self), to_bool(*base.args.front())) );
  };
}

// }}}
// custom_type {{{

value::base* fn_custom_type_ctr(vm::call_stack& base)
{
  check_size(1, base.args.size());
  if (base.args.front()->type == &type::custom_type)
    return base.args.front();
  throw std::runtime_error {
    "can only construct a Type from another Type"
  };
}

// }}}

}

// }}}
// Types {{{

value::builtin_type type::array {fn_array_ctr, {
  { {"size"},   {fn_array_size} },
  { {"append"}, {fn_array_append} },
  { {"at"},     {fn_array_at} }
}};

value::builtin_type type::integer{{fn_integer_ctr}, {
  { {"add"},            {fn_integer_op(std::plus<int>{})}           },
  { {"subtract"},       {fn_integer_op(std::minus<int>{})}          },
  { {"times"},          {fn_integer_op(std::multiplies<int>{})}     },
  { {"divides"},        {fn_integer_op(std::divides<int>{})}        },
  { {"modulo"},         {fn_integer_op(std::modulus<int>{})}        },
  { {"bitand"},         {fn_integer_op(std::bit_and<int>{})}        },
  { {"bitor"},          {fn_integer_op(std::bit_or<int>{})}         },
  { {"xor"},            {fn_integer_op(std::bit_xor<int>{})}        },
  { {"equals"},         {fn_int_bool_op(std::equal_to<int>{})}      },
  { {"unequal"},        {fn_int_bool_op(std::not_equal_to<int>{})}  },
  { {"less"},           {fn_int_bool_op(std::less<int>{})}          },
  { {"greater"},        {fn_int_bool_op(std::greater<int>{})}       },
  { {"less_equals"},    {fn_int_bool_op(std::less_equal<int>{})}    },
  { {"greater_equals"}, {fn_int_bool_op(std::greater_equal<int>{})} }
} };

value::builtin_type type::floating_point{{fn_floating_point_ctr}, {
  { {"equals"},         {fn_floating_point_equals}                        },
  { {"unequal"},        {fn_floating_point_unequal}                       },
  { {"add"},            {fn_floating_point_op(std::plus<double>{})}       },
  { {"subtract"},       {fn_floating_point_op(std::minus<double>{})}      },
  { {"times"},          {fn_floating_point_op(std::multiplies<double>{})} },
  { {"divides"},        {fn_floating_point_op(std::divides<double>{})}    },
  { {"less"},           {fn_float_bool_op(std::less<double>{})}           },
  { {"greater"},        {fn_float_bool_op(std::greater<double>{})}        },
  { {"less_equals"},    {fn_float_bool_op(std::less_equal<double>{})}     },
  { {"greater_equals"}, {fn_float_bool_op(std::greater_equal<double>{})}  }
}};

value::builtin_type type::string {{fn_string_ctr}, {
  { {"size"},    {fn_string_size}    },
  { {"append"},  {fn_string_append}  },
  { {"equals"},  {fn_string_equals}  },
  { {"unequal"}, {fn_string_unequal} }
}};

value::builtin_type type::symbol {{fn_symbol_ctr}, {
  { {"equals"},  {fn_symbol_equals}  },
  { {"unequal"}, {fn_symbol_unequal} },
  { {"to_str"},  {fn_symbol_to_str}  }
}};

value::builtin_type type::boolean {{fn_bool_ctr}, {
  { {"equals"},  {fn_bool_op(std::equal_to<bool>{})}     },
  { {"unequal"}, {fn_bool_op(std::not_equal_to<bool>{})} }
}};

value::builtin_type type::custom_type {{fn_custom_type_ctr}, {
}};

value::builtin_type type::nil      {nullptr, { }};
value::builtin_type type::function {nullptr, { }};

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
