#include "lang_utils.h"

#include "builtins.h"
#include "gc.h"
#include "value/boolean.h"

bool vv::truthy(const value::base* val)
{
  if (val->type == &builtin::type::nil)
    return false;
  else if (val->type == &builtin::type::boolean)
    return static_cast<const value::boolean*>(val)->val;
  return true;
}

vv::value::base* vv::throw_exception(const std::string& value, vm::machine& vm)
{
  vm.push_str(value);
  vm.except();
  return vm.retval;
}

bool vv::check_size(size_t expected, size_t receieved, vm::machine& vm)
{
  if (expected != receieved) {
    throw_exception("wrong number of arguments (expected "    +
                    std::to_string(expected)  += ", got " +
                    std::to_string(receieved) += ")",
                    vm);
    return false;
  }
  return true;
}

vv::value::base* vv::pop_arg(vm::machine& vm)
{
  auto arg = vm.stack->parent->pushed_args.back();
  vm.stack->parent->pushed_args.pop_back();
  return arg;
}

vv::value::base* vv::find_method(value::type* type, symbol name)
{
  while (&type->parent != type && !type->methods.count(name))
    type = static_cast<value::type*>(&type->parent);
  if (type->methods.count(name))
    return type->methods[name];

  return nullptr;
}
