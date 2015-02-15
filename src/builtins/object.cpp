#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_object_equals(vm::machine& vm)
{
  return gc::alloc<value::boolean>( vm.frame->self->equals(*get_arg(vm, 0)) );
}

value::base* fn_object_unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !vm.frame->self->equals(*get_arg(vm, 0)) );
}

value::base* fn_object_not(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !truthy(&*vm.frame->self) );
}

value::base* fn_object_type(vm::machine& vm)
{
  return vm.frame->self->type;
}

value::builtin_function obj_equals  {fn_object_equals,  1};
value::builtin_function obj_unequal {fn_object_unequal, 1};
value::builtin_function obj_not     {fn_object_not,     0};
value::builtin_function obj_type    {fn_object_type,    0};
}
value::type type::object {gc::alloc<value::base>, {
  { {"equals"},  &obj_equals },
  { {"unequal"}, &obj_unequal },
  { {"not"},     &obj_not },
  { {"type"},    &obj_type }
}, builtin::type::object, {"Object"}};

