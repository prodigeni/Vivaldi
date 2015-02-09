#ifndef LANG_UTILS_H
#define LANG_UTILS_H

#include "value.h"
#include "vm.h"

namespace vv {

bool truthy(const value::base* value);
value::base* throw_exception(const std::string& value, vm::machine& vm);
bool check_size(size_t expected, size_t receieved, vm::machine& vm);

}

#endif
