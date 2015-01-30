#ifndef IL_BUILTINS_H
#define IL_BUILTINS_H

#include "environment.h"
#include "value.h"

namespace il {

namespace builtin {

namespace function {

extern value::builtin_function gets;
extern value::builtin_function puts;
extern value::builtin_function quit;
extern value::builtin_function make_array;
extern value::builtin_function size;
extern value::builtin_function type;

}

namespace type {

extern value::custom_type string;
extern value::custom_type integer;
extern value::custom_type floating_point;
extern value::custom_type boolean;
extern value::custom_type array;
extern value::custom_type dictionary;
extern value::custom_type symbol;
extern value::custom_type iterator;
extern value::custom_type range;
extern value::custom_type nil;
extern value::custom_type custom_type;
extern value::custom_type custom_object;

}

}

}

#endif
