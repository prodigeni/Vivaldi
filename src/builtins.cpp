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

#include <iostream>

using namespace il;
using namespace builtin;

namespace {

value::base* fn_gets(const std::vector<value::base*>& args)
{
  check_size(0, args.size());
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

value::base* fn_puts(const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  //if (args.front()->type() == &type::string)
    //std::cout << static_cast<value::string*>(args.front())->str() << '\n';
  //else
    std::cout << args.front()->value() << '\n';
  return gc::alloc<value::nil>( );
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

value::builtin_function function::gets{fn_gets};
value::builtin_function function::puts{fn_puts};
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
  static_cast<value::array*>(self)->members().push_back(args.front());
  return self;
}

value::base* fn_array_at(value::base* self,
                         const std::vector<value::base*>& args)
{
  check_size(1, args.size());
  if (args.front()->type() != &type::integer)
    throw std::runtime_error{"index must be an integer"};
  auto val = static_cast<value::integer*>(args.front())->int_val();
  return static_cast<value::array*>(self)->members().at(val);
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
    return gc::alloc<value::boolean>( false );
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

}

value::builtin_type type::array {fn_array_ctr, {
  { {"size"},   fn_array_size },
  { {"append"}, fn_array_append },
  { {"at"},     fn_array_at }
}};

value::builtin_type type::integer{fn_integer_ctr, {
  { {std::string{"equals"}},         fn_integer_equals },
  { {std::string{"unequal"}},        fn_integer_unequal },
  { {std::string{"add"}},            fn_integer_op(std::plus<int>{}) },
  { {std::string{"subtract"}},       fn_integer_op(std::minus<int>{}) },
  { {std::string{"times"}},          fn_integer_op(std::multiplies<int>{}) },
  { {std::string{"divides"}},        fn_integer_op(std::divides<int>{}) },
  { {std::string{"modulo"}},         fn_integer_op(std::modulus<int>{}) },
  { {std::string{"bitand"}},         fn_integer_op(std::bit_and<int>{}) },
  { {std::string{"bitor"}},          fn_integer_op(std::bit_or<int>{}) },
  { {std::string{"xor"}},            fn_integer_op(std::bit_xor<int>{}) },
  { {std::string{"less"}},           fn_int_bool_op(std::less<int>{}) },
  { {std::string{"greater"}},        fn_int_bool_op(std::greater<int>{}) },
  { {std::string{"less_equals"}},    fn_int_bool_op(std::less_equal<int>{}) },
  { {std::string{"greater_equals"}}, fn_int_bool_op(std::greater_equal<int>{}) }
}};

value::builtin_type type::floating_point{fn_floating_point_ctr, {
  { {std::string{"equals"}},         fn_floating_point_equals },
  { {std::string{"unequal"}},        fn_floating_point_unequal },
  { {std::string{"add"}},            fn_floating_point_op(std::plus<double>{}) },
  { {std::string{"subtract"}},       fn_floating_point_op(std::minus<double>{}) },
  { {std::string{"times"}},          fn_floating_point_op(std::multiplies<double>{}) },
  { {std::string{"divides"}},        fn_floating_point_op(std::divides<double>{}) },
  { {std::string{"less"}},           fn_float_bool_op(std::less<double>{}) },
  { {std::string{"greater"}},        fn_float_bool_op(std::greater<double>{}) },
  { {std::string{"less_equals"}},    fn_float_bool_op(std::less_equal<double>{}) },
  { {std::string{"greater_equals"}}, fn_float_bool_op(std::greater_equal<double>{}) }
}};

value::builtin_type type::string {fn_string_ctr, {
  { {"size"},    fn_string_size },
  { {"append"},  fn_string_append },
  { {"equals"},  fn_string_equals },
  { {"unequal"}, fn_string_unequal }
}};
