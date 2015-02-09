#ifndef LANG_UTILS_H
#define LANG_UTILS_H

#include "value.h"

namespace vv {

bool truthy(const value::base* value);
void check_size(size_t expected, size_t receieved);

}

#endif
