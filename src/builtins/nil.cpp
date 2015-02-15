#include "builtins.h"

vv::value::type vv::builtin::type::nil {[]{ return nullptr; }, {
}, vv::builtin::type::object, {"Nil"}};
