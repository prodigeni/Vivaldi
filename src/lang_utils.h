#ifndef LANG_UTILS_H
#define LANG_UTILS_H

#include "value.h"
#include "vm.h"

namespace vv {

bool truthy(const value::base* value);

value::base* throw_exception(const std::string& value, vm::machine& vm);
value::base* get_arg(vm::machine& vm, size_t idx);

value::base* find_method(value::type* type, symbol name);

}

#endif
