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

auto custom_type_default_ctr_maker(value::type* type)
{
  return [=](vm::call_stack&) { return gc::alloc<value::base>( type ); };
}

value::base* fn_custom_type_ctr(vm::call_stack& base)
{
  std::unordered_map<symbol, value::base*> methods;
  for (auto i = base.args.size(); i--; --i) {
    auto name = to_symbol(*base.args[i]);
    auto definition = base.args[i - 1];
    methods[name] = definition;
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
