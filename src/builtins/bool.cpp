#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/boolean.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_bool_init(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type == &type::boolean)
    return arg;
  return gc::alloc<value::boolean>( truthy(arg) );
}

value::builtin_function bool_init {fn_bool_init, 1};

}

value::type type::boolean {gc::alloc<value::boolean>, {
  { {"init"}, &bool_init }
}, builtin::type::object, {"Bool"}};
