#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/custom_type.h"
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

value::base* fn_make_array(const std::vector<value::base*>& args)
{
  return gc::alloc<value::array>( args );
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
value::builtin_function function::make_array{fn_make_array};
value::builtin_function function::size{fn_size};
value::builtin_function function::type{fn_type};
