#include "function.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_type.h"

using namespace il;

value::function::function(vector_ref<vm::command> new_body,
                          std::shared_ptr<vm::call_stack> new_enclosure)
  : base      {&builtin::type::function},
    body      {new_body},
    enclosure {new_enclosure}
{ }

std::string value::function::value() const { return "<function>"; }
