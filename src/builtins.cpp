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

namespace {

value::base* fn_print(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() == &type::string)
    std::cout << static_cast<value::string*>(args.front())->str();
  else
    std::cout << args.front()->value();
  return gc::alloc<value::nil>( );
}

value::base* fn_puts(const std::vector<value::base*>& args)
{
  auto ret = gc::push_argument(fn_print(args));
  std::cout << '\n';
  gc::pop_argument();
  return ret;
}

value::base* fn_gets(const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

[[noreturn]] value::base* fn_quit(const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  exit(0);
}

value::base* fn_size(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  return args.front()->call_method(symbol{"size"}, {});
}

value::base* fn_type(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  return args.front()->type();
}

}

value::builtin_function function::print{fn_print};
value::builtin_function function::puts{fn_puts};
value::builtin_function function::gets{fn_gets};
value::builtin_function function::quit{fn_quit};
value::builtin_function function::size{fn_size};
value::builtin_function function::type{fn_type};

namespace {

int to_int(value::base* boxed)
{
  if (boxed->type() != &type::integer)
    throw std::runtime_error{"argument must be an integer"};
  return static_cast<value::integer*>(boxed)->int_val();
}

double to_float(value::base* boxed)
{
  if (boxed->type() != &type::floating_point)
    throw std::runtime_error{"argument must be a float"};
  return static_cast<value::floating_point*>(boxed)->float_val();
}

const std::string& to_string(value::base* boxed)
{
  if (boxed->type() != &type::string)
    throw std::runtime_error{"argument must be a string"};
  return static_cast<value::string*>(boxed)->str();
}

il::symbol to_symbol(value::base* boxed)
{
  if (boxed->type() != &type::symbol)
    throw std::runtime_error{"argument must be a symbol"};
  return static_cast<value::symbol*>(boxed)->sym();
}

// array {{{

value::base* fn_array_ctr(const std::vector<value::base*>& args)
{
  return gc::alloc<value::array>( args );
}

value::base* fn_array_size(value::base* self,
                           const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  auto sz = static_cast<value::array*>(self)->members().size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_array_append(value::base* self,
                             const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() == &type::array) {
    auto& arr = static_cast<value::array*>(self)->members();
    const auto& new_mems = static_cast<value::array*>(args.front())->members();
    copy(begin(new_mems), end(new_mems), back_inserter(arr));
  } else {
    static_cast<value::array*>(self)->members().push_back(args.front());
  }
  return self;
}

value::base* fn_array_at(value::base* self,
                         const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::integer)
    throw std::runtime_error{"index must be an integer"};
  auto val = static_cast<value::integer*>(args.front())->int_val();
  const auto& arr = static_cast<value::array*>(self)->members();
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    throw std::runtime_error{"out of range (expected 0-" + std::to_string(arr.size()) + ", got " + std::to_string(val) + ")"};
  return arr.at(val);
}

// }}}
// integer {{{

value::base* fn_integer_ctr(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  auto type = args.front()->type();

  if (type == &type::integer)
    return args.front();

  if (type == &type::floating_point)
    return gc::alloc<value::integer>(static_cast<int>(to_float(args.front())));

  throw std::runtime_error{"cannot create Integer from " + args.front()->value()};
}

value::base* fn_integer_equals(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::integer)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( to_int(self) == to_int(args.front()) );
}

value::base* fn_integer_unequal(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::integer)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>( to_int(self) != to_int(args.front()) );
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](value::base* self, const std::vector<value::base*>& args)
  {
    check_size(1, args.size());
    return gc::alloc<value::integer>( op(to_int(self), to_int(args.front())) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](value::base* self, const std::vector<value::base*>& args)
  {
    check_size(1, args.size());
    return gc::alloc<value::boolean>( op(to_int(self), to_int(args.front())) );
  };
}

// }}}
// floating_point {{{

value::base* fn_floating_point_ctr(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  auto type = args.front()->type();

  if (type == &type::floating_point)
    return args.front();

  if (type == &type::integer) {
    auto int_val = to_int(args.front());
    return gc::alloc<value::floating_point>( static_cast<double>(int_val) );
  }

  throw std::runtime_error{"cannot create Float from " + args.front()->value()};
}

value::base* fn_floating_point_equals(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::floating_point)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( to_float(self) == to_float(args.front()) );
}

value::base* fn_floating_point_unequal(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::floating_point)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( to_float(self) != to_float(args.front()) );
}

template <typename F>
auto fn_floating_point_op(const F& op)
{
  return [=](value::base* self, const std::vector<value::base*>& args)
  {
    check_size(1, args.size());
    return gc::alloc<value::floating_point>( op(to_float(self),
                                                to_float(args.front())) );
  };
}

template <typename F>
auto fn_float_bool_op(const F& op)
{
  return [=](value::base* self, const std::vector<value::base*>& args)
  {
    check_size(1, args.size());
    return gc::alloc<value::boolean>( op(to_float(self),
                                         to_float(args.front())) );
  };
}

// }}}
// string {{{

value::base* fn_string_ctr(const std::vector<value::base*>& args)
{
  throw std::runtime_error{"not yet implemented"};
}

value::base* fn_string_size(value::base* self,
                            const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  auto sz = static_cast<value::string*>(self)->str().size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_string_equals(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(to_string(self) == to_string(args.front()));
}

value::base* fn_string_unequal(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(to_string(self) != to_string(args.front()));
}

value::base* fn_string_append(value::base* self,
                              const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  auto str = to_string(args.front());
  static_cast<value::string*>(self)->str() += str;
  return self;
}

// }}}
// symbol {{{

value::base* fn_symbol_ctr(const std::vector<value::base*>& args)
{
  throw std::runtime_error{"not yet implemented"};
}

value::base* fn_symbol_equals(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(to_symbol(self) == to_symbol(args.front()));
}

value::base* fn_symbol_unequal(value::base* self,
                               const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(to_symbol(self) != to_symbol(args.front()));
}

value::base* fn_symbol_to_str(value::base* self,
                              const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  return gc::alloc<value::string>( to_string(to_symbol(self)) );
}

// }}}

}

value::builtin_type type::array {fn_array_ctr, {
  { {"size"},   fn_array_size },
  { {"append"}, fn_array_append },
  { {"at"},     fn_array_at }
}};

value::builtin_type type::integer{fn_integer_ctr, {
  { {"equals"},         fn_integer_equals },
  { {"unequal"},        fn_integer_unequal },
  { {"add"},            fn_integer_op(std::plus<int>{}) },
  { {"subtract"},       fn_integer_op(std::minus<int>{}) },
  { {"times"},          fn_integer_op(std::multiplies<int>{}) },
  { {"divides"},        fn_integer_op(std::divides<int>{}) },
  { {"modulo"},         fn_integer_op(std::modulus<int>{}) },
  { {"bitand"},         fn_integer_op(std::bit_and<int>{}) },
  { {"bitor"},          fn_integer_op(std::bit_or<int>{}) },
  { {"xor"},            fn_integer_op(std::bit_xor<int>{}) },
  { {"less"},           fn_int_bool_op(std::less<int>{}) },
  { {"greater"},        fn_int_bool_op(std::greater<int>{}) },
  { {"less_equals"},    fn_int_bool_op(std::less_equal<int>{}) },
  { {"greater_equals"}, fn_int_bool_op(std::greater_equal<int>{}) }
}};

value::builtin_type type::floating_point{fn_floating_point_ctr, {
  { {"equals"},         fn_floating_point_equals },
  { {"unequal"},        fn_floating_point_unequal },
  { {"add"},            fn_floating_point_op(std::plus<double>{}) },
  { {"subtract"},       fn_floating_point_op(std::minus<double>{}) },
  { {"times"},          fn_floating_point_op(std::multiplies<double>{}) },
  { {"divides"},        fn_floating_point_op(std::divides<double>{}) },
  { {"less"},           fn_float_bool_op(std::less<double>{}) },
  { {"greater"},        fn_float_bool_op(std::greater<double>{}) },
  { {"less_equals"},    fn_float_bool_op(std::less_equal<double>{}) },
  { {"greater_equals"}, fn_float_bool_op(std::greater_equal<double>{}) }
}};

value::builtin_type type::string {fn_string_ctr, {
  { {"size"},    fn_string_size },
  { {"append"},  fn_string_append },
  { {"equals"},  fn_string_equals },
  { {"unequal"}, fn_string_unequal }
}};

value::builtin_type type::symbol {fn_symbol_ctr, {
  { {"equals"},  fn_symbol_equals },
  { {"unequal"}, fn_symbol_unequal },
  { {"to_str"},  fn_symbol_to_str }
}};
value::builtin_type type::boolean     {nullptr, { }};
value::builtin_type type::nil         {nullptr, { }};
value::builtin_type type::custom_type {nullptr, { }};

environment builtin::g_base_env {{
  { {"print"},   &builtin::function::print },
  { {"puts"},    &builtin::function::puts },
  { {"gets"},    &builtin::function::gets },
  { {"quit"},    &builtin::function::quit },
  { {"size"},    &builtin::function::size },
  { {"type"},    &builtin::function::type },
  { {"Array"},   &builtin::type::array },
  { {"Float"},   &builtin::type::floating_point },
  { {"Integer"}, &builtin::type::integer },
  { {"String"},  &builtin::type::string },
  { {"Bool"},    &builtin::type::boolean },
  { {"Nil"},     &builtin::type::nil },
  { {"Symbol"},  &builtin::type::symbol },
  { {"Type"},    &builtin::type::custom_type }
}};
