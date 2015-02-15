#include "builtins.h"

#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

// custom_type {{{

value::base* fn_custom_type_parent(vm::machine& vm)
{
  return &static_cast<value::type&>(*vm.frame->self).parent;
}

// }}}

value::builtin_function custom_type_parent {fn_custom_type_parent, 0};

}
value::type type::custom_type {[]{ return nullptr; }, {
  { {"parent"}, &custom_type_parent }
}, builtin::type::object, {"Type"}};
