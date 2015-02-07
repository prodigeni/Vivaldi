#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "ast/function_definition.h"
#include "value/builtin_type.h"
#include "value/function.h"

using namespace il;

value::base::base(basic_type* new_type)
  : type     {new_type},
    m_marked {false}
{
  if (type)
    type->each_key([&](auto sym) { members[sym] = type->method(sym); });
}

void value::base::mark()
{
  m_marked = true;
  for (auto& i : members)
    if (!i.second->marked())
      i.second->mark();
}

value::basic_type::basic_type()
  : base {nullptr}
{ }
