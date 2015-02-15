#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/dictionary.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

// dictionary {{{

value::base* fn_dictionary_init(vm::machine& vm)
{
  auto dict = static_cast<value::dictionary*>(&*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::dictionary)
    return throw_exception("Dictionaries can only be constructed from other Dictionaries",
                           vm);
  dict->val = static_cast<value::dictionary*>( arg )->val;
  return dict;
}

value::base* fn_dictionary_size(vm::machine& vm)
{
  auto sz = static_cast<value::dictionary&>(*vm.frame->self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::base* fn_dictionary_at(vm::machine& vm)
{
  auto& dict = static_cast<value::dictionary&>(*vm.frame->self);
  auto arg = get_arg(vm, 0);
  if (!dict.val.count(arg))
    dict.val[arg] = gc::alloc<value::nil>( );
  return dict.val[arg];
}

value::base* fn_dictionary_set_at(vm::machine& vm)
{
  auto& dict = static_cast<value::dictionary&>(*vm.frame->self);
  auto arg = get_arg(vm, 0);
  return dict.val[arg] = get_arg(vm, 1);
}

// }}}

value::builtin_function dictionary_init   {fn_dictionary_init,   1};
value::builtin_function dictionary_size   {fn_dictionary_size,   0};
value::builtin_function dictionary_at     {fn_dictionary_at,     1};
value::builtin_function dictionary_set_at {fn_dictionary_set_at, 2};

}

value::type type::dictionary {gc::alloc<value::dictionary>, {
  { {"init"},   &dictionary_init },
  { {"size"},   &dictionary_size },
  { {"at"},     &dictionary_at },
  { {"set_at"}, &dictionary_set_at },
}, builtin::type::object, {"Dictionary"}};
