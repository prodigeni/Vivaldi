#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_function.h"
#include "value/symbol.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;

namespace {

const std::string& to_string(const value::base* boxed)
{
  return static_cast<const value::string*>(boxed)->val;
}

vv::symbol to_symbol(const value::base* boxed)
{
  return static_cast<const value::symbol*>(boxed)->val;
}

value::base* fn_symbol_init(vm::machine& vm)
{
  auto& sym = static_cast<value::symbol&>(*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::symbol)
    sym.val = to_symbol(arg);
  if (arg->type == &type::string)
    sym.val = vv::symbol{to_string(arg)};
  else
    return throw_exception("Symbols can only be constructed a String or another Symbol",
                           vm);
  return &sym;
}

value::base* fn_symbol_equals(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(to_symbol(&*vm.frame->self)==to_symbol(arg));
}

value::base* fn_symbol_unequal(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(to_symbol(&*vm.frame->self)!=to_symbol(arg));
}

// }}}

value::builtin_function symbol_init    {fn_symbol_init,    1};
value::builtin_function symbol_equals  {fn_symbol_equals,  1};
value::builtin_function symbol_unequal {fn_symbol_unequal, 1};

}

value::type type::symbol {gc::alloc<value::symbol>, {
  { {"init"},    &symbol_init    },
  { {"equals"},  &symbol_equals  },
  { {"unequal"}, &symbol_unequal },
}, builtin::type::object, {"Symbol"}};
